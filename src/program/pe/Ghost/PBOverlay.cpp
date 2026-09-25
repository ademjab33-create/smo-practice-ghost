#include "pe/Ghost/PBOverlay.h"
#include "nn/os.h"
#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include "pe/Menu/UserConfig.h"
#include <cstdio>

namespace pe {

PBOverlay::PBOverlay()
{
    mFrameCounter = 0;
    mNewPBFlashTimer = 0;
}

void PBOverlay::update()
{
    mFrameCounter++;
    if (GhostManager::instance()->isNewPBRecorded()) {
        mNewPBFlashTimer = 180; // Faire clignoter pendant ~3 secondes à 60 FPS
        GhostManager::instance()->clearNewPBRecorded();
    }

    if (mNewPBFlashTimer > 0) {
        mNewPBFlashTimer--;
    }
}

void PBOverlay::formatTime(s64 ticks, char* outBuffer, size_t bufferSize)
{
    if (!outBuffer || bufferSize == 0) return;

    if (ticks <= 0) {
        std::snprintf(outBuffer, bufferSize, "--:--.---");
        return;
    }

    s64 freq = nn::os::GetSystemTickFrequency();
    s64 totalSeconds = ticks / freq;
    s64 minutes = totalSeconds / 60;
    s64 seconds = totalSeconds % 60;
    s64 millis = (ticks % freq) * 1000 / freq;

    std::snprintf(outBuffer, bufferSize, "%02lld:%02lld.%03lld", minutes, seconds, millis);
}

void PBOverlay::draw()
{
    if (!getConfig()->mPBOverlayEnabled) {
        return;
    }

    auto* ilTracker = ILTracker::instance();
    auto* pbStorage = PBStorage::instance();
    auto* ghostMgr = GhostManager::instance();

    KingdomId kingdom = ilTracker->getCurrentKingdom();
    SegmentId segment = ilTracker->getCurrentSegment();

    const PBRecord* pb = pbStorage->getRecord(kingdom, segment);

    // Positionnement de l'overlay HUD en haut à droite de l'écran (ex: x=1000, y=30)
    ImVec2 basePos = getConfig()->mPBOverlayPos;
    float fontSize = getConfig()->mPBOverlayFontSize > 0 ? getConfig()->mPBOverlayFontSize : 22.0f;

    char titleBuf[128];
    std::snprintf(titleBuf, sizeof(titleBuf), "[%s - %s]", ilTracker->getCurrentKingdomName(), ilTracker->getCurrentSegmentName());

    char pbBuf[64];
    if (pb && pb->mBestTicks > 0) {
        char timeStr[32];
        formatTime(pb->mBestTicks, timeStr, sizeof(timeStr));
        std::snprintf(pbBuf, sizeof(pbBuf), "PB: %s", timeStr);
    } else {
        std::snprintf(pbBuf, sizeof(pbBuf), "PB: --:--.---");
    }

    char runBuf[64];
    char deltaBuf[64] = {0};
    ImU32 deltaColor = IM_COL32(255, 255, 255, 255);

    if (ilTracker->isRunActive()) {
        s64 currentTicks = ilTracker->getElapsedTicks();
        char curTimeStr[32];
        formatTime(currentTicks, curTimeStr, sizeof(curTimeStr));
        std::snprintf(runBuf, sizeof(runBuf), "TIME: %s", curTimeStr);

        // Calcul du Delta en direct par rapport au PB
        if (pb && pb->mBestTicks > 0) {
            s64 deltaTicks = currentTicks - pb->mBestTicks;
            s64 freq = nn::os::GetSystemTickFrequency();
            s64 deltaSec = deltaTicks / freq;
            s64 deltaMillis = ((deltaTicks < 0 ? -deltaTicks : deltaTicks) % freq) * 1000 / freq;

            if (deltaTicks < 0) {
                // En avance sur le PB : Vert
                std::snprintf(deltaBuf, sizeof(deltaBuf), "Δ -%02lld.%03lld", (-deltaSec), deltaMillis);
                deltaColor = IM_COL32(50, 255, 50, 255);
            } else {
                // En retard sur le PB : Rouge
                std::snprintf(deltaBuf, sizeof(deltaBuf), "Δ +%02lld.%03lld", deltaSec, deltaMillis);
                deltaColor = IM_COL32(255, 60, 60, 255);
            }
        }
    } else {
        std::snprintf(runBuf, sizeof(runBuf), "TIME: READY");
    }

    // Indicateur d'état du fantôme
    char ghostStatusBuf[64];
    ImU32 ghostColor = IM_COL32(150, 150, 150, 255);
    if (ghostMgr->isGhostEnabled()) {
        if (pb && pb->mHasGhostReplay) {
            std::snprintf(ghostStatusBuf, sizeof(ghostStatusBuf), "GHOST: ACTIVE (%.0f%%)", ghostMgr->getGhostAlpha() * 100.0f);
            ghostColor = IM_COL32(0, 220, 255, 255);
        } else {
            std::snprintf(ghostStatusBuf, sizeof(ghostStatusBuf), "GHOST: RECORDING");
            ghostColor = IM_COL32(255, 200, 0, 255);
        }
    } else {
        std::snprintf(ghostStatusBuf, sizeof(ghostStatusBuf), "GHOST: DISABLED");
    }

    // Dessin du fond translucide du HUD
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetIO().Fonts->Fonts[0];

    float boxWidth = 360.0f;
    float boxHeight = 110.0f;
    ImVec2 boxMin = basePos;
    ImVec2 boxMax = ImVec2(basePos.x + boxWidth, basePos.y + boxHeight);

    // Fond arrondi semi-transparent noir
    drawList->AddRectFilled(boxMin, boxMax, IM_COL32(15, 15, 20, 200), 8.0f);
    drawList->AddRect(boxMin, boxMax, IM_COL32(80, 80, 100, 180), 8.0f, 0, 1.5f);

    // Rendu des lignes de texte
    float textX = basePos.x + 12.0f;
    float lineY = basePos.y + 8.0f;

    // Titre Kingdom & Section
    drawList->AddText(font, fontSize * 0.9f, ImVec2(textX, lineY), IM_COL32(255, 215, 0, 255), titleBuf);
    lineY += fontSize + 4.0f;

    // Ligne PB
    drawList->AddText(font, fontSize, ImVec2(textX, lineY), IM_COL32(220, 220, 220, 255), pbBuf);

    // Statut Ghost (à droite de la ligne PB)
    drawList->AddText(font, fontSize * 0.8f, ImVec2(textX + 180.0f, lineY + 2.0f), ghostColor, ghostStatusBuf);
    lineY += fontSize + 4.0f;

    // Ligne Temps courant & Delta
    drawList->AddText(font, fontSize, ImVec2(textX, lineY), IM_COL32(255, 255, 255, 255), runBuf);
    if (deltaBuf[0] != '\0') {
        drawList->AddText(font, fontSize, ImVec2(textX + 220.0f, lineY), deltaColor, deltaBuf);
    }

    // Bannière flash en cas de nouveau record personnel (New PB!)
    if (mNewPBFlashTimer > 0) {
        bool flashOn = (mFrameCounter % 16) < 8;
        if (flashOn) {
            ImVec2 bannerMin = ImVec2(basePos.x - 20.0f, basePos.y - 35.0f);
            ImVec2 bannerMax = ImVec2(basePos.x + boxWidth + 20.0f, basePos.y - 5.0f);
            drawList->AddRectFilled(bannerMin, bannerMax, IM_COL32(255, 215, 0, 230), 6.0f);
            drawList->AddText(font, fontSize * 1.1f, ImVec2(basePos.x + 40.0f, basePos.y - 32.0f), IM_COL32(0, 0, 0, 255), "★ NEW PERSONAL BEST! ★");
        }
    }
}

} // namespace pe

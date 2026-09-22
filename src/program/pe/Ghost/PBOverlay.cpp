#include "pe/Ghost/PBOverlay.h"
#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include <imgui.h>

namespace pe {

void PBOverlay::formatTicks(s64 ticks, char* buf, int bufSize) {
    if (ticks <= 0) { buf[0] = '-'; buf[1] = '\0'; return; }
    s64 totalMs = (ticks * 1000) / 19200000LL;
    int ms = (int)(totalMs % 1000);
    int sec = (int)((totalMs / 1000) % 60);
    int min = (int)(totalMs / 60000);
    int pos = 0;
    if (min > 0) { if (min >= 10) buf[pos++] = '0' + (min/10); buf[pos++] = '0' + (min%10); buf[pos++] = ':'; }
    buf[pos++] = '0' + (sec/10); buf[pos++] = '0' + (sec%10); buf[pos++] = '.';
    buf[pos++] = '0' + (ms/100); buf[pos++] = '0' + ((ms/10)%10); buf[pos++] = '0' + (ms%10); buf[pos] = '\0';
}

void PBOverlay::draw() {
    auto* gm = GhostManager::instance();
    auto* tr = ILTracker::instance();
    auto* st = PBStorage::instance();
    if (!gm || !tr || !st) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.65f);
    if (ImGui::Begin("##PB", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        ImGui::TextColored(ImVec4(0.3f,0.85f,1,1), "%s", tr->getCurrentKingdomName());
        const PBRecord* pb = st->getRecord(tr->getCurrentKingdom(), tr->getCurrentSegment());
        if (gm->isRunActive()) {
            ImGui::Separator();
            char t[32]; formatTicks(gm->getElapsedTicks(), t, 32);
            ImGui::Text("Timer: %s", t);
            if (pb) {
                char p[32]; formatTicks(pb->mBestTicks, p, 32);
                s64 d = gm->getElapsedTicks() - pb->mBestTicks;
                char db[32];
                if (d >= 0) { formatTicks(d, db, 32); ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "PB %s +%s", p, db); }
                else { formatTicks(-d, db, 32); ImGui::TextColored(ImVec4(0.3f,1,0.3f,1), "PB %s -%s", p, db); }
            }
        } else {
            if (pb) { char p[32]; formatTicks(pb->mBestTicks, p, 32); ImGui::Text("PB: %s", p); }
            if (gm->isNewPB()) ImGui::TextColored(ImVec4(1,0.85f,0,1), "NEW PB!");
            ImGui::TextDisabled("R+Up=Start R+Down=Stop");
        }
    }
    ImGui::End();
}

} // namespace pe
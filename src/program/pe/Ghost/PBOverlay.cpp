#include "pe/Ghost/PBOverlay.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include <imgui.h>

namespace pe {

void PBOverlay::formatTicks(s64 ticks, char* buf, int bufSize) {
    if (ticks <= 0) { buf[0] = '-'; buf[1] = '\0'; return; }
    s64 freq = 19200000LL; // Switch tick frequency
    s64 totalMs = (ticks * 1000) / freq;
    int ms = (int)(totalMs % 1000);
    int sec = (int)((totalMs / 1000) % 60);
    int min = (int)(totalMs / 60000);

    // Manual snprintf-like formatting
    int pos = 0;
    if (min > 0) {
        if (min >= 10) buf[pos++] = '0' + (min / 10);
        buf[pos++] = '0' + (min % 10);
        buf[pos++] = ':';
    }
    buf[pos++] = '0' + (sec / 10);
    buf[pos++] = '0' + (sec % 10);
    buf[pos++] = '.';
    buf[pos++] = '0' + (ms / 100);
    buf[pos++] = '0' + ((ms / 10) % 10);
    buf[pos++] = '0' + (ms % 10);
    buf[pos] = '\0';
}

void PBOverlay::draw() {
    auto* tracker = ILTracker::instance();
    auto* storage = PBStorage::instance();
    if (!tracker || !storage) return;

    KingdomId kingdom = tracker->getCurrentKingdom();
    SegmentId segment = tracker->getCurrentSegment();
    if (kingdom == KingdomId::Unknown) return;

    const PBRecord* pb = storage->getRecord(kingdom, segment);

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.6f);

    if (ImGui::Begin("PB Tracker", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "%s", tracker->getCurrentKingdomName());
        ImGui::Separator();

        if (tracker->isRunActive()) {
            char timeBuf[32];
            formatTicks(tracker->getElapsedTicks(), timeBuf, sizeof(timeBuf));
            ImGui::Text("Current: %s", timeBuf);

            if (pb) {
                char pbBuf[32];
                formatTicks(pb->mBestTicks, pbBuf, sizeof(pbBuf));
                ImGui::Text("PB:      %s", pbBuf);

                s64 delta = tracker->getElapsedTicks() - pb->mBestTicks;
                char deltaBuf[32];
                if (delta > 0) {
                    formatTicks(delta, deltaBuf, sizeof(deltaBuf));
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "+%s", deltaBuf);
                } else {
                    formatTicks(-delta, deltaBuf, sizeof(deltaBuf));
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "-%s", deltaBuf);
                }
            }
        } else {
            if (pb) {
                char pbBuf[32];
                formatTicks(pb->mBestTicks, pbBuf, sizeof(pbBuf));
                ImGui::Text("PB: %s (%s)", pbBuf, tracker->getCurrentSegmentName());
            } else {
                ImGui::TextDisabled("No PB recorded");
            }
        }
    }
    ImGui::End();
}

} // namespace pe
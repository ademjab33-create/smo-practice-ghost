#pragma once

#include "basis/seadTypes.h"
#include "imgui.h"
#include "pe/Menu/IComponent.h"

namespace pe {

class PBOverlay : public IComponent {
public:
    PBOverlay();
    ~PBOverlay() = default;

    void update() override;
    void draw() override;

    static void formatTime(s64 ticks, char* outBuffer, size_t bufferSize);

private:
    int mFrameCounter = 0;
    int mNewPBFlashTimer = 0;
};

} // namespace pe

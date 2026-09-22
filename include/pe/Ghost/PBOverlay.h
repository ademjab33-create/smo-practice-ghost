#pragma once
#include "pe/Ghost/GhostTypes.h"

namespace pe {

class PBOverlay {
public:
    static void draw();
    static void formatTicks(s64 ticks, char* buf, int bufSize);
};

} // namespace pe
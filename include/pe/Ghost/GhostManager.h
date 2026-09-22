#pragma once
#include "pe/Ghost/GhostTypes.h"
#include <heap/seadDisposer.h>

namespace al { class Scene; }

namespace pe {

class GhostManager {
    SEAD_SINGLETON_DISPOSER(GhostManager);
    GhostManager();
    ~GhostManager();

public:
    void startRun();
    void stopRun();
    void resetRun();
    void update();

    bool isRunActive() const { return mIsRunActive; }
    s64 getElapsedTicks() const;
    u32 getElapsedFrames() const { return mElapsedFrames; }
    bool isNewPB() const { return mIsNewPB; }
    void clearNewPB() { mIsNewPB = false; }

private:
    bool mIsRunActive = false;
    s64 mStartTick = 0;
    s64 mEndTick = 0;
    u32 mElapsedFrames = 0;
    bool mIsNewPB = false;
};

} // namespace pe
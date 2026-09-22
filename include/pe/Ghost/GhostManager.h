#pragma once
#include "pe/Ghost/GhostTypes.h"
#include <heap/seadDisposer.h>

namespace al { class Scene; class ActorInitInfo; }

namespace pe {

class GhostPuppetActor;

class GhostManager {
    SEAD_SINGLETON_DISPOSER(GhostManager);
    GhostManager();
    ~GhostManager();

public:
    void initPuppet(const al::ActorInitInfo& info);
    void startRun();
    void stopRun();
    void resetRun();
    void update(al::Scene* scene);

    bool isRunActive() const { return mIsRunActive; }
    s64 getElapsedTicks() const;
    u32 getElapsedFrames() const { return mElapsedFrames; }
    bool isNewPB() const { return mIsNewPB; }
    void clearNewPB() { mIsNewPB = false; }

private:
    void recordFrame(al::Scene* scene);
    void playbackFrame();
    void saveGhostData();
    void loadGhostData();

    bool mIsRunActive = false;
    s64 mStartTick = 0;
    s64 mEndTick = 0;
    u32 mElapsedFrames = 0;
    bool mIsNewPB = false;

    GhostPuppetActor* mPuppet = nullptr;
    GhostFrame* mRecordBuf = nullptr;
    GhostFrame* mPlayBuf = nullptr;
    int mRecordCount = 0;
    int mPlayCount = 0;
    int mPlayStep = 0;
};

} // namespace pe
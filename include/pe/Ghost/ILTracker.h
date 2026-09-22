#pragma once
#include "pe/Ghost/GhostTypes.h"
#include <heap/seadDisposer.h>

namespace al { class Scene; }

namespace pe {

class ILTracker {
    SEAD_SINGLETON_DISPOSER(ILTracker);
    ILTracker();
    ~ILTracker();

public:
    void init();
    void update(al::Scene* scene);
    void startRun();
    void endRun();
    void resetRun();
    s64 getElapsedTicks() const;

    void notifyMoonStory();
    void notifyBlackScreen();
    void notifyOdyssey();
    void notifyMoonEnd();
    void notifyCapEnd();

    KingdomId getCurrentKingdom() const { return mCurrentKingdom; }
    SegmentId getCurrentSegment() const { return mCurrentSegment; }
    const char* getCurrentKingdomName() const;
    const char* getCurrentSegmentName() const;
    bool isRunActive() const { return mIsRunActive; }
    u32 getElapsedFrames() const { return mElapsedFrames; }

private:
    void detectKingdom(const char* stageName);
    void checkSplitConditions();

    KingdomId mCurrentKingdom = KingdomId::Unknown;
    SegmentId mCurrentSegment = SegmentId::FullKingdom;
    char mCurrentStageName[128] = {};
    bool mIsRunActive = false;
    s64 mStartTick = 0;
    s64 mEndTick = 0;
    u32 mElapsedFrames = 0;
    int mMoonStoryCount = 0;
    int mBlackScreenCount = 0;
    int mOdysseyCount = 0;
    int mMoonEndCount = 0;
    int mCapEndCount = 0;
};

} // namespace pe
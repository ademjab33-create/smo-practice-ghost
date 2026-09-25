#pragma once

#include "basis/seadTypes.h"
#include "pe/Ghost/GhostTypes.h"
#include <sead/heap/seadDisposer.h>

namespace al {
class Scene;
}

namespace pe {

class ILTracker {
    SEAD_SINGLETON_DISPOSER(ILTracker);
    ILTracker();
    ~ILTracker();

public:
    void init();
    void update(al::Scene* scene);

    // Détection du Royaume et du Segment
    KingdomId getCurrentKingdom() const { return mCurrentKingdom; }
    SegmentId getCurrentSegment() const { return mCurrentSegment; }
    const char* getCurrentKingdomName() const;
    const char* getCurrentSegmentName() const;

    // Événements déclencheurs (Triggers)
    void onStageEntry();
    void notifyMoonStory();
    void notifyBlackScreen();
    void notifyOdyssey();
    void notifyMoonEnd();
    void notifyCapEnd();

    // Contrôle manuel / Timer
    void startRun();
    void endRun();
    void resetRun();

    // Statut
    bool isRunActive() const { return mIsRunActive; }
    s64 getElapsedTicks() const;
    u32 getElapsedFrames() const { return mElapsedFrames; }

private:
    void detectKingdomAndSegment(al::Scene* scene);
    void checkSplitConditions();

    KingdomId mCurrentKingdom = KingdomId::Unknown;
    SegmentId mCurrentSegment = SegmentId::FullKingdom;
    char mCurrentStageName[64] = {0};

    bool mIsRunActive = false;
    s64 mStartTick = 0;
    s64 mEndTick = 0;
    u32 mElapsedFrames = 0;

    // Compteurs de composants pour les règles de split
    int mMoonStoryCount = 0;
    int mBlackScreenCount = 0;
    int mOdysseyCount = 0;
    int mMoonEndCount = 0;
    int mCapEndCount = 0;
};

} // namespace pe

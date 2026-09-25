#pragma once

#include "basis/seadTypes.h"
#include "pe/Ghost/GhostPuppetActor.h"
#include "pe/Ghost/GhostReplayData.h"
#include "pe/Ghost/GhostTypes.h"
#include <sead/heap/seadDisposer.h>

namespace al {
class Scene;
struct ActorInitInfo;
}

namespace pe {

enum class GhostState {
    Idle,
    Playing,
    Recording,
    RecordingAndPlaying
};

class GhostManager {
    SEAD_SINGLETON_DISPOSER(GhostManager);
    GhostManager();
    ~GhostManager();

public:
    void init(const al::ActorInitInfo& info);
    void update(al::Scene* scene);

    // Contrôle du cycle de run
    void onRunStart(KingdomId kingdom, SegmentId segment, const char* segmentName);
    void onRunEnd(s64 elapsedTicks, u32 totalFrames);
    void onRunReset();

    // Configuration
    bool isGhostEnabled() const { return mIsEnabled; }
    void setGhostEnabled(bool enabled);

    float getGhostAlpha() const;
    void setGhostAlpha(float alpha);

    // État
    GhostState getState() const { return mState; }
    bool isNewPBRecorded() const { return mIsNewPBNotification; }
    void clearNewPBRecorded() { mIsNewPBNotification = false; }

    const GhostReplayData& getLoadedPBData() const { return mLoadedPBData; }
    const GhostReplayData& getCurrentRunData() const { return mCurrentRunData; }

    GhostPuppetActor* getPuppet() const { return mGhostPuppet; }

private:
    void recordCurrentPlayerFrame(al::Scene* scene, u32 step);
    void updateGhostPlayback(u32 step);

    bool mIsEnabled = true;
    GhostState mState = GhostState::Idle;
    u32 mCurrentStep = 0;

    KingdomId mActiveKingdom = KingdomId::Unknown;
    SegmentId mActiveSegment = SegmentId::FullKingdom;
    char mActiveSegmentName[64] = {0};

    GhostPuppetActor* mGhostPuppet = nullptr;
    GhostReplayData mLoadedPBData;      // Replay du PB existant à rejouer
    GhostReplayData mCurrentRunData;     // Enregistrement de la tentative en cours

    bool mIsNewPBNotification = false;
};

} // namespace pe

#pragma once

#include "al/Library/LiveActor/LiveActor.h"
#include "pe/Ghost/GhostTypes.h"
#include <sead/heap/seadDisposer.h>

namespace al {
struct ActorInitInfo;
}

namespace pe {

class GhostPuppetActor : public al::LiveActor, public sead::IDisposer {
public:
    GhostPuppetActor(const char* name);
    ~GhostPuppetActor() override;

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void control() override;
    void movement() override;
    void makeActorAlive() override;
    void makeActorDead() override;

    void setGhostAlpha(float alpha);
    float getGhostAlpha() const { return mGhostAlpha; }

    void applyReplayFrame(const ReplayFrame& frame);
    void startMarioAction(const char* actionName);
    void startCapAction(const char* actionName);

    al::LiveActor* getCapActor() const { return mCapActor; }

private:
    void applyShaderTransparency();

    al::LiveActor* mCapActor = nullptr;
    float mGhostAlpha = 0.45f;
    char mCurrentMarioAnim[64] = {0};
    char mCurrentCapAnim[64] = {0};
    bool mIsCapVisible = true;
};

} // namespace pe

#pragma once
#include "al/Library/LiveActor/LiveActor.h"
namespace al { class ActorInitInfo; }

namespace pe {
class GhostPuppetActor : public al::LiveActor {
public:
    GhostPuppetActor(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void control() override;
    void applyPosition(const sead::Vector3f& pos);
    void setGhostAlpha(float alpha);
private:
    float mGhostAlpha = 0.45f;
};
} // namespace pe
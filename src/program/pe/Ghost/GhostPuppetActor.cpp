#include "pe/Ghost/GhostPuppetActor.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorClippingFunction.h"
#include "al/Library/LiveActor/ActorModelFunction.h"
#include "al/Library/LiveActor/ActorSensorFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include <prim/seadSafeString.h>

namespace al {
    class ActorInitInfo;
    void initActorWithArchiveName(al::LiveActor* actor, const al::ActorInitInfo& initInfo, const sead::SafeString& archiveName, const char* suffix);
}

namespace pe {

GhostPuppetActor::GhostPuppetActor(const char* name) : al::LiveActor(name) {
    mGhostAlpha = 0.45f;
}

void GhostPuppetActor::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, sead::SafeString("PlayerActorHakoniwa"), nullptr);
    al::invalidateHitSensors(this);
    al::invalidateClipping(this);
    al::hideSilhouetteModelIfShow(this);
    al::setModelAlphaMask(this, mGhostAlpha);
    makeActorDead();
}

void GhostPuppetActor::control() {}

void GhostPuppetActor::applyPosition(const sead::Vector3f& pos) {
    al::setTrans(this, pos);
}

void GhostPuppetActor::setGhostAlpha(float alpha) {
    if (alpha < 0.05f) alpha = 0.05f;
    if (alpha > 1.0f) alpha = 1.0f;
    mGhostAlpha = alpha;
    al::setModelAlphaMask(this, mGhostAlpha);
}

} // namespace pe
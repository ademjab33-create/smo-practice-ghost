#include "pe/Ghost/GhostPuppetActor.h"
#include "al/Library/LiveActor/ActorActionFunction.h"
#include "al/Library/LiveActor/ActorClippingFunction.h"
#include "al/Library/LiveActor/ActorCollisionFunction.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorModelFunction.h"
namespace al {
    class ActorInitInfo;
    void initActorWithArchiveName(al::LiveActor* actor, const al::ActorInitInfo& initInfo, const sead::SafeString& archiveName, const char* suffix);
}
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorSensorUtil.h"
#include "al/Library/LiveActor/SubActorKeeper.h"
#include <cstring>

namespace pe {

GhostPuppetActor::GhostPuppetActor(const char* name)
    : al::LiveActor(name)
{
    mGhostAlpha = 0.45f;
    mIsCapVisible = true;
}

GhostPuppetActor::~GhostPuppetActor()
{
}

void GhostPuppetActor::init(const al::ActorInitInfo& initInfo)
{
    // Initialisation du modèle Mario principal
    al::initActorWithArchiveName(this, initInfo, "PlayerActorHakoniwa", nullptr);

    // Création de l'acteur Cappy pour le fantôme
    mCapActor = new al::LiveActor("GhostCapActor");
    al::initActorWithArchiveName(mCapActor, initInfo, "CapManHeroEyesMarker", nullptr);

    // Désactivation complète des collisions et hitboxes pour éviter d'interférer avec le vrai Mario
    al::invalidateHitSensors(this);
    al::invalidateHitSensors(mCapActor);

    // Empêcher le culling de clipping à distance du fantôme
    al::invalidateClipping(this);
    al::invalidateClipping(mCapActor);

    // Masquer la silhouette standard (le fantôme est déjà translucide)
    al::hideSilhouetteModelIfShow(this);
    al::hideSilhouetteModelIfShow(mCapActor);

    // Appliquer le shader de transparence fantôme
    applyShaderTransparency();

    // Démarrer initialement masqué
    makeActorDead();
}

void GhostPuppetActor::initAfterPlacement()
{
    al::LiveActor::initAfterPlacement();
    if (mCapActor) {
        mCapActor->initAfterPlacement();
    }
}

void GhostPuppetActor::control()
{
    // Pas de logique physique ni de gravité pour le fantôme :
    // Toutes ses coordonnées sont dictées strictement par la frame du replay
}

void GhostPuppetActor::movement()
{
    al::LiveActor::movement();
    if (mCapActor && al::isAlive(mCapActor)) {
        mCapActor->movement();
    }
}

void GhostPuppetActor::makeActorAlive()
{
    al::LiveActor::makeActorAlive();
    if (mCapActor && mIsCapVisible) {
        mCapActor->makeActorAlive();
    }
    applyShaderTransparency();
}

void GhostPuppetActor::makeActorDead()
{
    al::LiveActor::makeActorDead();
    if (mCapActor) {
        mCapActor->makeActorDead();
    }
}

void GhostPuppetActor::setGhostAlpha(float alpha)
{
    if (alpha < 0.05f) alpha = 0.05f;
    if (alpha > 1.0f) alpha = 1.0f;
    mGhostAlpha = alpha;
    applyShaderTransparency();
}

void GhostPuppetActor::applyShaderTransparency()
{
    // Appliquer le masque alpha sur le corps de Mario
    al::setModelAlphaMask(this, mGhostAlpha);

    // Synchroniser l'alpha sur les sous-acteurs (tête, mains, etc.)
    al::SubActorKeeper* subKeeper = getSubActorKeeper();
    if (subKeeper) {
        for (int i = 0; i < subKeeper->getSubActorNum(); i++) {
            al::LiveActor* subActor = subKeeper->getSubActor(i);
            if (subActor) {
                al::setModelAlphaMask(subActor, mGhostAlpha);
            }
        }
    }

    // Appliquer la transparence sur Cappy
    if (mCapActor) {
        al::setModelAlphaMask(mCapActor, mGhostAlpha);
    }
}

void GhostPuppetActor::applyReplayFrame(const ReplayFrame& frame)
{
    // Position et orientation Mario
    al::setTrans(this, frame.mPlayerTrans);
    al::setQuat(this, frame.mPlayerQuat);

    // Blending squelettique
    for (int i = 0; i < 6; i++) {
        al::setSklAnimBlendWeight(this, frame.mBlendWeights[i], i);
    }

    // Position et orientation Cappy
    if (mCapActor) {
        al::setTrans(mCapActor, frame.mCapTrans);
        al::setQuat(mCapActor, frame.mCapQuat);

        // Gestion de la visibilité de Cappy
        if (frame.mIsCapVisible != mIsCapVisible) {
            mIsCapVisible = frame.mIsCapVisible;
            if (mIsCapVisible) {
                if (al::isDead(mCapActor)) mCapActor->makeActorAlive();
            } else {
                if (al::isAlive(mCapActor)) mCapActor->makeActorDead();
            }
        }
    }
}

void GhostPuppetActor::startMarioAction(const char* actionName)
{
    if (!actionName || actionName[0] == '\0') return;
    if (std::strcmp(mCurrentMarioAnim, actionName) != 0) {
        std::strncpy(mCurrentMarioAnim, actionName, sizeof(mCurrentMarioAnim) - 1);
        al::tryStartActionIfNotPlaying(this, actionName);
    }
}

void GhostPuppetActor::startCapAction(const char* actionName)
{
    if (!mCapActor || !actionName || actionName[0] == '\0') return;
    if (std::strcmp(mCurrentCapAnim, actionName) != 0) {
        std::strncpy(mCurrentCapAnim, actionName, sizeof(mCurrentCapAnim) - 1);
        al::tryStartActionIfNotPlaying(mCapActor, actionName);
    }
}

} // namespace pe

#include "pe/Ghost/GhostManager.h"
#include "Player/PlayerActorHakoniwa.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include "al/Library/LiveActor/ActorActionFunction.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/Scene/Scene.h"
#include "pe/Ghost/PBStorage.h"
#include "pe/Menu/Menu.h"
#include "rs/Util/PlayerUtil.h"
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(GhostManager);

GhostManager::GhostManager()
{
    mIsEnabled = true;
    mState = GhostState::Idle;
    mCurrentStep = 0;
    mIsNewPBNotification = false;
}

GhostManager::~GhostManager()
{
    if (mGhostPuppet) {
        delete mGhostPuppet;
        mGhostPuppet = nullptr;
    }
}

void GhostManager::init(const al::ActorInitInfo& info)
{
    if (!mGhostPuppet) {
        mGhostPuppet = new GhostPuppetActor("GhostReplayMario");
        mGhostPuppet->init(info);
    }
}

void GhostManager::setGhostEnabled(bool enabled)
{
    mIsEnabled = enabled;
    if (!mIsEnabled && mGhostPuppet) {
        mGhostPuppet->makeActorDead();
    }
}

float GhostManager::getGhostAlpha() const
{
    if (mGhostPuppet) {
        return mGhostPuppet->getGhostAlpha();
    }
    return 0.45f;
}

void GhostManager::setGhostAlpha(float alpha)
{
    if (mGhostPuppet) {
        mGhostPuppet->setGhostAlpha(alpha);
    }
}

void GhostManager::onRunStart(KingdomId kingdom, SegmentId segment, const char* segmentName)
{
    mActiveKingdom = kingdom;
    mActiveSegment = segment;
    if (segmentName) {
        std::strncpy(mActiveSegmentName, segmentName, sizeof(mActiveSegmentName) - 1);
    }

    mCurrentStep = 0;
    mIsNewPBNotification = false;

    // Réinitialiser le buffer de la nouvelle tentative
    mCurrentRunData.reset();
    mCurrentRunData.getHeader().mKingdomId = kingdom;
    mCurrentRunData.getHeader().mSegmentId = segment;
    if (segmentName) {
        std::strncpy(mCurrentRunData.getHeader().mSegmentName, segmentName, sizeof(mCurrentRunData.getHeader().mSegmentName) - 1);
    }

    // Charger le replay du PB existant s'il existe sur la carte SD
    char ghostPath[256];
    PBStorage::instance()->getGhostFilePath(kingdom, segment, ghostPath, sizeof(ghostPath));
    bool hasPBReplay = mLoadedPBData.loadFromFile(ghostPath, getMenuHeap());

    if (mIsEnabled && hasPBReplay && mGhostPuppet) {
        mGhostPuppet->makeActorAlive();
        mState = GhostState::RecordingAndPlaying;

        // Positionner le fantôme sur la première frame
        updateGhostPlayback(0);
    } else {
        if (mGhostPuppet) {
            mGhostPuppet->makeActorDead();
        }
        mState = GhostState::Recording;
    }
}

void GhostManager::update(al::Scene* scene)
{
    if (mState == GhostState::Idle || !scene || !scene->mIsAlive) {
        return;
    }

    // 1. Enregistrement de la frame actuelle du joueur
    if (mState == GhostState::Recording || mState == GhostState::RecordingAndPlaying) {
        recordCurrentPlayerFrame(scene, mCurrentStep);
    }

    // 2. Relecture et synchronisation de la frame du fantôme
    if (mState == GhostState::RecordingAndPlaying || mState == GhostState::Playing) {
        updateGhostPlayback(mCurrentStep);
    }

    mCurrentStep++;
}

void GhostManager::recordCurrentPlayerFrame(al::Scene* scene, u32 step)
{
    PlayerActorHakoniwa* player = reinterpret_cast<PlayerActorHakoniwa*>(rs::getPlayerActor(scene));
    if (!player) return;

    ReplayFrame frame;
    frame.mStep = step;
    frame.mPlayerTrans = al::getTrans(player);
    frame.mPlayerQuat = al::getQuat(player);

    // Récupération de Cappy
    if (player->mHackCap) {
        frame.mCapTrans = al::getTrans(player->mHackCap);
        frame.mCapQuat = al::getQuat(player->mHackCap);
        if (player->mHackCap->mJointKeeper) {
            frame.mCapJoint = player->mHackCap->mJointKeeper->mJointRot;
            frame.mCapSkew = player->mHackCap->mJointKeeper->mSkew;
        }
        frame.mIsCapVisible = al::isAlive(player->mHackCap);
    }

    // Récupération des poids de squelette
    if (player->mPlayerAnimator) {
        for (int i = 0; i < 6; i++) {
            frame.mBlendWeights[i] = player->mPlayerAnimator->getBlendWeight(i);
        }
    }

    // Mode 2D
    frame.mIs2D = rs::isPlayer2D(player);

    // Inputs manette
    frame.mButtons = al::getPadHold(-1);

    mCurrentRunData.appendFrame(frame);
}

void GhostManager::updateGhostPlayback(u32 step)
{
    if (!mGhostPuppet || !mIsEnabled) return;

    if (step < mLoadedPBData.getFrameCount()) {
        const ReplayFrame* frame = mLoadedPBData.getFrame(step);
        if (frame) {
            if (al::isDead(mGhostPuppet)) {
                mGhostPuppet->makeActorAlive();
            }
            mGhostPuppet->applyReplayFrame(*frame);
        }
    } else {
        // Fin du replay du fantôme : le joueur est en retard ou le fantôme a déjà fini
        mGhostPuppet->makeActorDead();
    }
}

void GhostManager::onRunEnd(s64 elapsedTicks, u32 totalFrames)
{
    if (mState == GhostState::Idle) return;

    // Arrêter le fantôme
    if (mGhostPuppet) {
        mGhostPuppet->makeActorDead();
    }

    mCurrentRunData.getHeader().mTotalTicks = elapsedTicks;
    mCurrentRunData.getHeader().mTotalFrames = totalFrames;

    // Comparer et mettre à jour le PB dans PBStorage
    bool isNewPB = false;
    PBStorage::instance()->updateRecordIfBetter(mActiveKingdom, mActiveSegment, mActiveSegmentName, elapsedTicks, totalFrames, isNewPB);

    if (isNewPB) {
        mIsNewPBNotification = true;

        // Sauvegarder la nouvelle tentative comme nouveau fichier fantôme PB
        char ghostPath[256];
        PBStorage::instance()->getGhostFilePath(mActiveKingdom, mActiveSegment, ghostPath, sizeof(ghostPath));
        mCurrentRunData.saveToFile(ghostPath);

        // Recharger immédiatement en tant que nouveau PB actif
        mLoadedPBData.loadFromFile(ghostPath, getMenuHeap());
    }

    mState = GhostState::Idle;
}

void GhostManager::onRunReset()
{
    if (mGhostPuppet) {
        mGhostPuppet->makeActorDead();
    }
    mCurrentRunData.reset();
    mState = GhostState::Idle;
    mCurrentStep = 0;
}

} // namespace pe

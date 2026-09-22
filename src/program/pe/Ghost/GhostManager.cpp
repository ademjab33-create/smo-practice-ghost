#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/GhostPuppetActor.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "rs/Util/PlayerUtil.h"
#include "nn/os.h"
#include "nn/fs.h"
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(GhostManager);

GhostManager::GhostManager() {
    mRecordBuf = new GhostFrame[kMaxGhostFrames];
    mPlayBuf = new GhostFrame[kMaxGhostFrames];
}

GhostManager::~GhostManager() {
    delete[] mRecordBuf;
    delete[] mPlayBuf;
}

void GhostManager::initPuppet(const al::ActorInitInfo& info) {
    if (!mPuppet) {
        mPuppet = new GhostPuppetActor("GhostPuppet");
        mPuppet->init(info);
    }
}

void GhostManager::startRun() {
    mStartTick = nn::os::GetSystemTick().m_tick;
    mEndTick = mStartTick;
    mElapsedFrames = 0;
    mRecordCount = 0;
    mIsRunActive = true;
    mIsNewPB = false;
    mPlayStep = 0;

    loadGhostData();

    if (mPuppet && mPlayCount > 0) {
        mPuppet->makeActorAlive();
    }
}

void GhostManager::stopRun() {
    if (!mIsRunActive) return;
    mEndTick = nn::os::GetSystemTick().m_tick;
    mIsRunActive = false;

    if (mPuppet && al::isAlive(mPuppet)) {
        mPuppet->makeActorDead();
    }

    auto* tracker = ILTracker::instance();
    auto* storage = PBStorage::instance();
    if (tracker && storage) {
        bool newPB = false;
        storage->updateRecordIfBetter(
            tracker->getCurrentKingdom(), tracker->getCurrentSegment(),
            tracker->getCurrentSegmentName(), getElapsedTicks(), mElapsedFrames, newPB);
        if (newPB) {
            mIsNewPB = true;
            saveGhostData();
        }
    }
}

void GhostManager::resetRun() {
    mIsRunActive = false;
    mStartTick = 0; mEndTick = 0; mElapsedFrames = 0;
    mRecordCount = 0; mPlayStep = 0;
    mIsNewPB = false;
    if (mPuppet && al::isAlive(mPuppet)) {
        mPuppet->makeActorDead();
    }
}

void GhostManager::update(al::Scene* scene) {
    if (!mIsRunActive) return;
    mElapsedFrames++;
    recordFrame(scene);
    playbackFrame();
}

void GhostManager::recordFrame(al::Scene* scene) {
    if (!scene || mRecordCount >= kMaxGhostFrames) return;
    PlayerActorBase* player = rs::getPlayerActor(scene);
    if (!player) return;
    sead::Vector3f pos = al::getTrans(player);
    mRecordBuf[mRecordCount].x = pos.x;
    mRecordBuf[mRecordCount].y = pos.y;
    mRecordBuf[mRecordCount].z = pos.z;
    mRecordCount++;
}

void GhostManager::playbackFrame() {
    if (!mPuppet || mPlayCount <= 0) return;
    if (mPlayStep < mPlayCount) {
        if (al::isDead(mPuppet)) mPuppet->makeActorAlive();
        sead::Vector3f pos(mPlayBuf[mPlayStep].x, mPlayBuf[mPlayStep].y, mPlayBuf[mPlayStep].z);
        mPuppet->applyPosition(pos);
        mPlayStep++;
    } else {
        if (al::isAlive(mPuppet)) mPuppet->makeActorDead();
    }
}

void GhostManager::saveGhostData() {
    auto* tracker = ILTracker::instance();
    if (!tracker || mRecordCount <= 0) return;

    char path[256];
    auto* storage = PBStorage::instance();
    if (storage) {
        storage->getGhostFilePath(tracker->getCurrentKingdom(), tracker->getCurrentSegment(), path, sizeof(path));
        nn::fs::FileHandle handle;
        s64 size = sizeof(int) + mRecordCount * sizeof(GhostFrame);
        nn::fs::CreateFile(path, size);
        if (nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Write).IsSuccess()) {
            nn::fs::SetFileSize(handle, size);
            nn::fs::WriteFile(handle, 0, &mRecordCount, sizeof(int), nn::fs::WriteOption());
            nn::fs::WriteFile(handle, sizeof(int), mRecordBuf, mRecordCount * sizeof(GhostFrame), nn::fs::WriteOption());
            nn::fs::FlushFile(handle);
            nn::fs::CloseFile(handle);
        }
    }
}

void GhostManager::loadGhostData() {
    mPlayCount = 0;
    mPlayStep = 0;
    auto* tracker = ILTracker::instance();
    auto* storage = PBStorage::instance();
    if (!tracker || !storage) return;

    char path[256];
    storage->getGhostFilePath(tracker->getCurrentKingdom(), tracker->getCurrentSegment(), path, sizeof(path));
    nn::fs::FileHandle handle;
    if (nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Read).IsSuccess()) {
        int count = 0;
        nn::fs::ReadFile(handle, 0, &count, sizeof(int));
        if (count > 0 && count <= kMaxGhostFrames) {
            nn::fs::ReadFile(handle, sizeof(int), mPlayBuf, count * sizeof(GhostFrame));
            mPlayCount = count;
        }
        nn::fs::CloseFile(handle);
    }
}

s64 GhostManager::getElapsedTicks() const {
    if (mIsRunActive) return nn::os::GetSystemTick().m_tick - mStartTick;
    return mEndTick - mStartTick;
}

} // namespace pe
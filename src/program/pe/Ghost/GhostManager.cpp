#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include "nn/os.h"

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(GhostManager);

GhostManager::GhostManager() {}
GhostManager::~GhostManager() {}

void GhostManager::startRun() {
    mStartTick = nn::os::GetSystemTick().m_tick;
    mEndTick = mStartTick;
    mElapsedFrames = 0;
    mIsRunActive = true;
    mIsNewPB = false;
}

void GhostManager::stopRun() {
    if (!mIsRunActive) return;
    mEndTick = nn::os::GetSystemTick().m_tick;
    mIsRunActive = false;

    auto* tracker = ILTracker::instance();
    auto* storage = PBStorage::instance();
    if (tracker && storage) {
        bool newPB = false;
        storage->updateRecordIfBetter(
            tracker->getCurrentKingdom(),
            tracker->getCurrentSegment(),
            tracker->getCurrentSegmentName(),
            getElapsedTicks(), mElapsedFrames, newPB);
        mIsNewPB = newPB;
    }
}

void GhostManager::resetRun() {
    mIsRunActive = false;
    mStartTick = 0;
    mEndTick = 0;
    mElapsedFrames = 0;
    mIsNewPB = false;
}

void GhostManager::update() {
    if (mIsRunActive) mElapsedFrames++;
}

s64 GhostManager::getElapsedTicks() const {
    if (mIsRunActive) return nn::os::GetSystemTick().m_tick - mStartTick;
    return mEndTick - mStartTick;
}

} // namespace pe
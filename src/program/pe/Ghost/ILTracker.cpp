#include "pe/Ghost/ILTracker.h"
#include "System/GameDataHolder.h"
#include "al/Library/Scene/Scene.h"
#include "Scene/SceneObjUtil.h"
#include "nn/os.h"
#include "pe/Ghost/GhostManager.h"
#include "pe/Menu/Timer.h"
#include "pe/Menu/UserConfig.h"
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(ILTracker);

ILTracker::ILTracker()
{
}

ILTracker::~ILTracker()
{
}

void ILTracker::init()
{
    mCurrentKingdom = KingdomId::Unknown;
    mCurrentSegment = SegmentId::FullKingdom;
    mIsRunActive = false;
    mStartTick = 0;
    mEndTick = 0;
    mElapsedFrames = 0;
    resetRun();
}

void ILTracker::detectKingdomAndSegment(al::Scene* scene)
{
    if (!scene || !scene->mIsAlive) return;

    GameDataHolder* holder = reinterpret_cast<GameDataHolder*>(al::getSceneObj(scene, 20));
    if (!holder) return;

    const char* stageName = holder->tryGetCurrentStageName();
    if (!stageName) return;

    KingdomId oldKingdom = mCurrentKingdom;

    // Détection du Royaume et de la sous-section selon le nom de scène interne
    if (std::strstr(stageName, "CapWorld")) {
        mCurrentKingdom = KingdomId::Cap;
        mCurrentSegment = SegmentId::CapEnd;
    } else if (std::strstr(stageName, "WaterfallWorld")) {
        mCurrentKingdom = KingdomId::Cascade;
        mCurrentSegment = SegmentId::CascadeOdyssey;
    } else if (std::strstr(stageName, "SandWorld")) {
        mCurrentKingdom = KingdomId::Sand;
        mCurrentSegment = SegmentId::SandOdyssey;
    } else if (std::strstr(stageName, "LakeWorld")) {
        mCurrentKingdom = KingdomId::Lake;
        mCurrentSegment = SegmentId::LakeOdyssey;
    } else if (std::strstr(stageName, "ForestWorld")) {
        mCurrentKingdom = KingdomId::Wooded;
        mCurrentSegment = SegmentId::WoodedOdyssey;
    } else if (std::strstr(stageName, "CloudWorld")) {
        mCurrentKingdom = KingdomId::Cloud;
        mCurrentSegment = SegmentId::CloudSkip;
    } else if (std::strstr(stageName, "ClashWorld")) {
        mCurrentKingdom = KingdomId::Lost;
        mCurrentSegment = SegmentId::LostOdyssey;
    } else if (std::strstr(stageName, "CityWorld")) {
        mCurrentKingdom = KingdomId::Metro;
        mCurrentSegment = SegmentId::MetroOdyssey;
    } else if (std::strstr(stageName, "SnowWorld")) {
        mCurrentKingdom = KingdomId::Snow;
        mCurrentSegment = SegmentId::SnowOdyssey;
    } else if (std::strstr(stageName, "SeaWorld")) {
        mCurrentKingdom = KingdomId::Seaside;
        mCurrentSegment = SegmentId::SeasideOdyssey;
    } else if (std::strstr(stageName, "LavaWorld")) {
        mCurrentKingdom = KingdomId::Luncheon;
        mCurrentSegment = SegmentId::LuncheonOdyssey;
    } else if (std::strstr(stageName, "BossRaidWorld")) {
        mCurrentKingdom = KingdomId::Ruined;
        mCurrentSegment = SegmentId::RuinedOdyssey;
    } else if (std::strstr(stageName, "SkyWorld")) {
        mCurrentKingdom = KingdomId::Bowser;
        mCurrentSegment = SegmentId::BowserOdyssey;
    } else if (std::strstr(stageName, "Special1World") || std::strstr(stageName, "RabbitRidge")) {
        mCurrentKingdom = KingdomId::DarkSide;
        mCurrentSegment = SegmentId::BowserBunnies;
    } else if (std::strstr(stageName, "Special2World")) {
        mCurrentKingdom = KingdomId::DarkerSide;
        mCurrentSegment = SegmentId::FullKingdom;
    } else if (std::strstr(stageName, "MoonWorld")) {
        mCurrentKingdom = KingdomId::Moon;
        mCurrentSegment = SegmentId::MoonEnd;
    } else if (std::strstr(stageName, "PeachWorld")) {
        mCurrentKingdom = KingdomId::Mushroom;
        mCurrentSegment = SegmentId::FullKingdom;
    }

    if (std::strcmp(mCurrentStageName, stageName) != 0) {
        std::strncpy(mCurrentStageName, stageName, sizeof(mCurrentStageName) - 1);
        onStageEntry();

        // Si le Timer Auto Kingdom est activé, démarrer le timer au début de chaque royaume
        auto* cfg = getConfig();
        if (cfg && cfg->mTimerAutoKingdom && cfg->mTimerEnabled) {
            if (mCurrentKingdom != KingdomId::Unknown && mCurrentKingdom != oldKingdom) {
                if (Timer::sInstance) {
                    Timer::sInstance->reset();
                    Timer::sInstance->start();
                }
            }
        }
    }
}

const char* ILTracker::getCurrentKingdomName() const
{
    switch (mCurrentKingdom) {
    case KingdomId::Cap: return "Cap Kingdom";
    case KingdomId::Cascade: return "Cascade Kingdom";
    case KingdomId::Sand: return "Sand Kingdom";
    case KingdomId::Lake: return "Lake Kingdom";
    case KingdomId::Wooded: return "Wooded Kingdom";
    case KingdomId::Cloud: return "Cloud Kingdom";
    case KingdomId::Lost: return "Lost Kingdom";
    case KingdomId::Metro: return "Metro Kingdom";
    case KingdomId::Snow: return "Snow Kingdom";
    case KingdomId::Seaside: return "Seaside Kingdom";
    case KingdomId::Luncheon: return "Luncheon Kingdom";
    case KingdomId::Ruined: return "Ruined Kingdom";
    case KingdomId::Bowser: return "Bowser's Kingdom";
    case KingdomId::Moon: return "Moon Kingdom";
    case KingdomId::DarkSide: return "Dark Side (Rabbit Ridge)";
    case KingdomId::DarkerSide: return "Darker Side";
    case KingdomId::Mushroom: return "Mushroom Kingdom";
    default: return "Unknown Kingdom";
    }
}

const char* ILTracker::getCurrentSegmentName() const
{
    switch (mCurrentSegment) {
    case SegmentId::BowserBunnies: return "Bunnies";
    case SegmentId::BowserOdyssey: return "Bowsers";
    case SegmentId::MoonEnd: return "Moon";
    case SegmentId::CapEnd: return "Cap End";
    case SegmentId::CascadeOdyssey: return "Cascade";
    case SegmentId::SandOdyssey: return "Sand";
    case SegmentId::LakeOdyssey: return "Lake";
    case SegmentId::WoodedOdyssey: return "Wooded";
    case SegmentId::CloudSkip: return "Cloud";
    case SegmentId::LostOdyssey: return "Lost";
    case SegmentId::MetroOdyssey: return "Metro";
    case SegmentId::SnowOdyssey: return "Snow";
    case SegmentId::SeasideOdyssey: return "Seaside";
    case SegmentId::LuncheonOdyssey: return "Luncheon";
    case SegmentId::RuinedOdyssey: return "Ruined";
    default: return "Full Kingdom";
    }
}

void ILTracker::update(al::Scene* scene)
{
    detectKingdomAndSegment(scene);

    if (mIsRunActive) {
        mElapsedFrames++;
    }
}

void ILTracker::onStageEntry()
{
    mMoonStoryCount = 0;
    mBlackScreenCount = 0;
    mOdysseyCount = 0;
    mMoonEndCount = 0;
    mCapEndCount = 0;
}

void ILTracker::startRun()
{
    mStartTick = nn::os::GetSystemTick().m_tick;
    mEndTick = mStartTick;
    mElapsedFrames = 0;
    mIsRunActive = true;

    mMoonStoryCount = 0;
    mBlackScreenCount = 0;
    mOdysseyCount = 0;
    mMoonEndCount = 0;
    mCapEndCount = 0;

    auto* ghostMgr = GhostManager::instance();
    if (ghostMgr) {
        ghostMgr->onRunStart(mCurrentKingdom, mCurrentSegment, getCurrentSegmentName());
    }
}

void ILTracker::endRun()
{
    if (!mIsRunActive && !(Timer::sInstance && Timer::sInstance->isRunning())) return;

    mEndTick = nn::os::GetSystemTick().m_tick;
    mIsRunActive = false;

    // Arrête le timer à la fin du royaume selon les règles SMO AutoSplit
    if (Timer::sInstance) {
        Timer::sInstance->stop();
    }

    auto* ghostMgr = GhostManager::instance();
    if (ghostMgr) {
        ghostMgr->onRunEnd(getElapsedTicks(), mElapsedFrames);
    }
}

void ILTracker::resetRun()
{
    mIsRunActive = false;
    mStartTick = 0;
    mEndTick = 0;
    mElapsedFrames = 0;

    mMoonStoryCount = 0;
    mBlackScreenCount = 0;
    mOdysseyCount = 0;
    mMoonEndCount = 0;
    mCapEndCount = 0;

    auto* ghostMgr = GhostManager::instance();
    if (ghostMgr) {
        ghostMgr->onRunReset();
    }
}

s64 ILTracker::getElapsedTicks() const
{
    if (mIsRunActive) {
        return nn::os::GetSystemTick().m_tick - mStartTick;
    }
    return mEndTick - mStartTick;
}

void ILTracker::notifyMoonStory()
{
    mMoonStoryCount++;
    checkSplitConditions();
}

void ILTracker::notifyBlackScreen()
{
    mBlackScreenCount++;
    checkSplitConditions();
}

void ILTracker::notifyOdyssey()
{
    mOdysseyCount++;
    checkSplitConditions();
}

void ILTracker::notifyMoonEnd()
{
    mMoonEndCount++;
    checkSplitConditions();
}

void ILTracker::notifyCapEnd()
{
    mCapEndCount++;
    checkSplitConditions();
}

void ILTracker::checkSplitConditions()
{
    bool shouldSplit = false;

    if (mCurrentSegment == SegmentId::BowserBunnies) {
        if (mMoonStoryCount >= 3 && mBlackScreenCount >= 1) {
            shouldSplit = true;
        }
    } else if (mCurrentSegment == SegmentId::BowserOdyssey) {
        if (mOdysseyCount >= 1 && mBlackScreenCount >= 1) {
            shouldSplit = true;
        }
    } else if (mCurrentSegment == SegmentId::MoonEnd) {
        if (mMoonEndCount >= 1) {
            shouldSplit = true;
        }
    } else if (mCurrentSegment == SegmentId::CapEnd) {
        if (mCapEndCount >= 1 && mBlackScreenCount >= 1) {
            shouldSplit = true;
        }
    } else {
        if (mOdysseyCount >= 1 && mBlackScreenCount >= 1) {
            shouldSplit = true;
        }
    }

    if (shouldSplit) {
        endRun();
    }
}

} // namespace pe

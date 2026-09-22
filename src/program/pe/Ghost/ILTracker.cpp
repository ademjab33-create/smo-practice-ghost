#include "pe/Ghost/ILTracker.h"
#include "nn/os.h"
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(ILTracker);

ILTracker::ILTracker() {}
ILTracker::~ILTracker() {}

void ILTracker::init() {
    mCurrentKingdom = KingdomId::Unknown;
    mCurrentSegment = SegmentId::FullKingdom;
    mIsRunActive = false;
    mStartTick = 0;
    mEndTick = 0;
    mElapsedFrames = 0;
}

void ILTracker::detectKingdom(const char* stageName) {
    if (!stageName) return;
    if (std::strstr(stageName, "CapWorld")) { mCurrentKingdom = KingdomId::Cap; mCurrentSegment = SegmentId::CapEnd; }
    else if (std::strstr(stageName, "WaterfallWorld")) { mCurrentKingdom = KingdomId::Cascade; mCurrentSegment = SegmentId::CascadeOdyssey; }
    else if (std::strstr(stageName, "SandWorld")) { mCurrentKingdom = KingdomId::Sand; mCurrentSegment = SegmentId::SandOdyssey; }
    else if (std::strstr(stageName, "LakeWorld")) { mCurrentKingdom = KingdomId::Lake; mCurrentSegment = SegmentId::LakeOdyssey; }
    else if (std::strstr(stageName, "ForestWorld")) { mCurrentKingdom = KingdomId::Wooded; mCurrentSegment = SegmentId::WoodedOdyssey; }
    else if (std::strstr(stageName, "CloudWorld")) { mCurrentKingdom = KingdomId::Cloud; mCurrentSegment = SegmentId::CloudSkip; }
    else if (std::strstr(stageName, "ClashWorld")) { mCurrentKingdom = KingdomId::Lost; mCurrentSegment = SegmentId::LostOdyssey; }
    else if (std::strstr(stageName, "CityWorld")) { mCurrentKingdom = KingdomId::Metro; mCurrentSegment = SegmentId::MetroOdyssey; }
    else if (std::strstr(stageName, "SnowWorld")) { mCurrentKingdom = KingdomId::Snow; mCurrentSegment = SegmentId::SnowOdyssey; }
    else if (std::strstr(stageName, "SeaWorld")) { mCurrentKingdom = KingdomId::Seaside; mCurrentSegment = SegmentId::SeasideOdyssey; }
    else if (std::strstr(stageName, "LavaWorld")) { mCurrentKingdom = KingdomId::Luncheon; mCurrentSegment = SegmentId::LuncheonOdyssey; }
    else if (std::strstr(stageName, "BossRaidWorld")) { mCurrentKingdom = KingdomId::Ruined; mCurrentSegment = SegmentId::RuinedOdyssey; }
    else if (std::strstr(stageName, "SkyWorld")) { mCurrentKingdom = KingdomId::Bowser; mCurrentSegment = SegmentId::BowserOdyssey; }
    else if (std::strstr(stageName, "MoonWorld")) { mCurrentKingdom = KingdomId::Moon; mCurrentSegment = SegmentId::MoonEnd; }
    else if (std::strstr(stageName, "Special1World")) { mCurrentKingdom = KingdomId::DarkSide; mCurrentSegment = SegmentId::BowserBunnies; }
    else if (std::strstr(stageName, "Special2World")) { mCurrentKingdom = KingdomId::DarkerSide; mCurrentSegment = SegmentId::FullKingdom; }
    else if (std::strstr(stageName, "PeachWorld")) { mCurrentKingdom = KingdomId::Mushroom; mCurrentSegment = SegmentId::FullKingdom; }
}

const char* ILTracker::getCurrentKingdomName() const {
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
    case KingdomId::DarkSide: return "Dark Side";
    case KingdomId::DarkerSide: return "Darker Side";
    case KingdomId::Mushroom: return "Mushroom Kingdom";
    default: return "Unknown";
    }
}

const char* ILTracker::getCurrentSegmentName() const {
    switch (mCurrentSegment) {
    case SegmentId::BowserBunnies: return "Bunnies";
    case SegmentId::BowserOdyssey: return "Bowsers";
    case SegmentId::MoonEnd: return "Moon";
    case SegmentId::CapEnd: return "Cap";
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

void ILTracker::update(al::Scene* scene) {
    if (mIsRunActive) mElapsedFrames++;
}

void ILTracker::startRun() {
    mStartTick = nn::os::GetSystemTick().m_tick;
    mEndTick = mStartTick;
    mElapsedFrames = 0;
    mIsRunActive = true;
    mMoonStoryCount = 0;
    mBlackScreenCount = 0;
    mOdysseyCount = 0;
    mMoonEndCount = 0;
    mCapEndCount = 0;
}

void ILTracker::endRun() {
    if (!mIsRunActive) return;
    mEndTick = nn::os::GetSystemTick().m_tick;
    mIsRunActive = false;
}

void ILTracker::resetRun() {
    mIsRunActive = false;
    mStartTick = 0;
    mEndTick = 0;
    mElapsedFrames = 0;
}

s64 ILTracker::getElapsedTicks() const {
    if (mIsRunActive) return nn::os::GetSystemTick().m_tick - mStartTick;
    return mEndTick - mStartTick;
}

void ILTracker::notifyMoonStory() { mMoonStoryCount++; checkSplitConditions(); }
void ILTracker::notifyBlackScreen() { mBlackScreenCount++; checkSplitConditions(); }
void ILTracker::notifyOdyssey() { mOdysseyCount++; checkSplitConditions(); }
void ILTracker::notifyMoonEnd() { mMoonEndCount++; checkSplitConditions(); }
void ILTracker::notifyCapEnd() { mCapEndCount++; checkSplitConditions(); }

void ILTracker::checkSplitConditions() {
    if (!mIsRunActive) return;
    bool shouldSplit = false;
    if (mCurrentSegment == SegmentId::BowserBunnies && mMoonStoryCount >= 3 && mBlackScreenCount >= 1) shouldSplit = true;
    else if (mCurrentSegment == SegmentId::BowserOdyssey && mOdysseyCount >= 1 && mBlackScreenCount >= 1) shouldSplit = true;
    else if (mCurrentSegment == SegmentId::MoonEnd && mMoonEndCount >= 1) shouldSplit = true;
    else if (mCurrentSegment == SegmentId::CapEnd && mCapEndCount >= 1 && mBlackScreenCount >= 1) shouldSplit = true;
    else if (mOdysseyCount >= 1 && mBlackScreenCount >= 1) shouldSplit = true;
    if (shouldSplit) endRun();
}

} // namespace pe
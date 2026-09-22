#pragma once
#include <types.h>

namespace pe {

enum class KingdomId : u8 {
    Unknown = 0, Cap, Cascade, Sand, Lake, Wooded, Cloud, Lost,
    Metro, Snow, Seaside, Luncheon, Ruined, Bowser, Moon, DarkSide, DarkerSide, Mushroom
};

enum class SegmentId : u8 {
    FullKingdom = 0, BowserBunnies, BowserOdyssey, MoonEnd, CapEnd,
    CascadeOdyssey, SandOdyssey, LakeOdyssey, WoodedOdyssey, CloudSkip,
    LostOdyssey, MetroOdyssey, SnowOdyssey, SeasideOdyssey, LuncheonOdyssey, RuinedOdyssey
};

struct GhostFrame {
    float x, y, z;
};

struct GhostHeader {
    u32 mMagic = 0x47485354;
    u32 mVersion = 1;
    KingdomId mKingdomId = KingdomId::Unknown;
    SegmentId mSegmentId = SegmentId::FullKingdom;
    char mSegmentName[32] = {};
    s64 mTotalTicks = 0;
    u32 mTotalFrames = 0;
};

struct PBRecord {
    KingdomId mKingdomId = KingdomId::Unknown;
    SegmentId mSegmentId = SegmentId::FullKingdom;
    char mSegmentName[32] = {};
    s64 mBestTicks = 0;
    u32 mBestFrames = 0;
};

static constexpr int kMaxGhostFrames = 36000;

} // namespace pe
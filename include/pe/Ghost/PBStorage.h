#pragma once
#include "pe/Ghost/GhostTypes.h"
#include <heap/seadDisposer.h>

namespace pe {

class PBStorage {
    SEAD_SINGLETON_DISPOSER(PBStorage);
    PBStorage();
    ~PBStorage();

public:
    void init();
    void updateRecordIfBetter(KingdomId kingdom, SegmentId segment, const char* segName, s64 ticks, u32 frames, bool& outIsNewPB);
    const PBRecord* getRecord(KingdomId kingdom, SegmentId segment) const;
    void getGhostFilePath(KingdomId kingdom, SegmentId segment, char* outPath, int pathLen) const;

private:
    void loadFromSD();
    void saveToSD();
    static constexpr int kMaxRecords = 64;
    PBRecord mRecords[kMaxRecords];
    int mRecordCount = 0;
};

} // namespace pe
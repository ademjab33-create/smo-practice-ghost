#pragma once

#include "basis/seadTypes.h"
#include "pe/Ghost/GhostTypes.h"
#include <sead/heap/seadDisposer.h>

namespace pe {

class PBStorage {
    SEAD_SINGLETON_DISPOSER(PBStorage);
    PBStorage();
    ~PBStorage();

public:
    static constexpr int MAX_RECORDS = 64;
    static constexpr const char* BASE_DIR = "sd:/smo/practice";
    static constexpr const char* GHOSTS_DIR = "sd:/smo/practice/ghosts";
    static constexpr const char* PBS_DIR = "sd:/smo/practice/pbs";

    void init();
    void setProfileId(u64 profileId);
    u64 getProfileId() const { return mProfileId; }

    const PBRecord* getRecord(KingdomId kingdom, SegmentId segment) const;
    PBRecord* getOrCreateRecord(KingdomId kingdom, SegmentId segment, const char* name);

    bool updateRecordIfBetter(KingdomId kingdom, SegmentId segment, const char* name, s64 elapsedTicks, u32 totalFrames, bool& outIsNewPB);
    void resetRecord(KingdomId kingdom, SegmentId segment);

    void getGhostFilePath(KingdomId kingdom, SegmentId segment, char* outBuffer, size_t bufferSize) const;

    bool load();
    bool save();

private:
    void ensureDirectories();
    int findRecordIndex(KingdomId kingdom, SegmentId segment) const;

    u64 mProfileId = 0x1000000000000001ULL; // ID de profil par défaut
    PBRecord mRecords[MAX_RECORDS];
    int mRecordCount = 0;
};

} // namespace pe

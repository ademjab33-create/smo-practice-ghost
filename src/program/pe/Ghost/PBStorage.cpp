#include "pe/Ghost/PBStorage.h"
#include "nn/fs.h"
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(PBStorage);

PBStorage::PBStorage() {}
PBStorage::~PBStorage() {}

void PBStorage::init() {
    nn::fs::CreateDirectory("sd:/PracticeMod");
    nn::fs::CreateDirectory("sd:/PracticeMod/Ghost");
    mRecordCount = 0;
    loadFromSD();
}

void PBStorage::loadFromSD() {
    nn::fs::FileHandle handle;
    if (nn::fs::OpenFile(&handle, "sd:/PracticeMod/Ghost/pb_records.bin", nn::fs::OpenMode_Read).IsSuccess()) {
        s64 fileSize = 0;
        nn::fs::GetFileSize(&fileSize, handle);
        int count = (int)(fileSize / sizeof(PBRecord));
        if (count > kMaxRecords) count = kMaxRecords;
        if (count > 0) {
            nn::fs::ReadFile(handle, 0, mRecords, count * sizeof(PBRecord));
            mRecordCount = count;
        }
        nn::fs::CloseFile(handle);
    }
}

void PBStorage::saveToSD() {
    nn::fs::FileHandle handle;
    nn::fs::CreateFile("sd:/PracticeMod/Ghost/pb_records.bin", mRecordCount * sizeof(PBRecord));
    if (nn::fs::OpenFile(&handle, "sd:/PracticeMod/Ghost/pb_records.bin", nn::fs::OpenMode_Write).IsSuccess()) {
        nn::fs::SetFileSize(handle, mRecordCount * sizeof(PBRecord));
        nn::fs::WriteFile(handle, 0, mRecords, mRecordCount * sizeof(PBRecord), nn::fs::WriteOption());
        nn::fs::FlushFile(handle);
        nn::fs::CloseFile(handle);
    }
}

void PBStorage::updateRecordIfBetter(KingdomId kingdom, SegmentId segment, const char* segName, s64 ticks, u32 frames, bool& outIsNewPB) {
    outIsNewPB = false;
    for (int i = 0; i < mRecordCount; i++) {
        if (mRecords[i].mKingdomId == kingdom && mRecords[i].mSegmentId == segment) {
            if (ticks < mRecords[i].mBestTicks) {
                mRecords[i].mBestTicks = ticks;
                mRecords[i].mBestFrames = frames;
                outIsNewPB = true;
                saveToSD();
            }
            return;
        }
    }
    if (mRecordCount < kMaxRecords) {
        PBRecord& r = mRecords[mRecordCount++];
        r.mKingdomId = kingdom;
        r.mSegmentId = segment;
        if (segName) std::strncpy(r.mSegmentName, segName, sizeof(r.mSegmentName) - 1);
        r.mBestTicks = ticks;
        r.mBestFrames = frames;
        outIsNewPB = true;
        saveToSD();
    }
}

const PBRecord* PBStorage::getRecord(KingdomId kingdom, SegmentId segment) const {
    for (int i = 0; i < mRecordCount; i++) {
        if (mRecords[i].mKingdomId == kingdom && mRecords[i].mSegmentId == segment)
            return &mRecords[i];
    }
    return nullptr;
}

void PBStorage::getGhostFilePath(KingdomId kingdom, SegmentId segment, char* outPath, int pathLen) const {
    // e.g. "sd:/PracticeMod/Ghost/ghost_3_5.bin"
    char buf[64];
    buf[0] = '\0';
    int k = (int)kingdom, s = (int)segment;
    // Manual int to string
    char numK[8], numS[8];
    int i = 0;
    if (k == 0) numK[i++] = '0'; else { int tmp = k; int digits[8]; int d = 0; while(tmp > 0) { digits[d++] = tmp % 10; tmp /= 10; } for(int j = d-1; j >= 0; j--) numK[i++] = '0' + digits[j]; }
    numK[i] = '\0';
    i = 0;
    if (s == 0) numS[i++] = '0'; else { int tmp = s; int digits[8]; int d = 0; while(tmp > 0) { digits[d++] = tmp % 10; tmp /= 10; } for(int j = d-1; j >= 0; j--) numS[i++] = '0' + digits[j]; }
    numS[i] = '\0';
    
    std::strncpy(outPath, "sd:/PracticeMod/Ghost/ghost_", pathLen - 1);
    std::strncat(outPath, numK, pathLen - std::strlen(outPath) - 1);
    std::strncat(outPath, "_", pathLen - std::strlen(outPath) - 1);
    std::strncat(outPath, numS, pathLen - std::strlen(outPath) - 1);
    std::strncat(outPath, ".bin", pathLen - std::strlen(outPath) - 1);
}

} // namespace pe
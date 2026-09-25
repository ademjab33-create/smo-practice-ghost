#include "pe/Ghost/PBStorage.h"
#include "helpers/fsHelper.h"
#include "nn/fs.hpp"
#include <cstdio>
#include <cstring>

namespace pe {

SEAD_SINGLETON_DISPOSER_IMPL(PBStorage);

PBStorage::PBStorage()
{
    mRecordCount = 0;
    std::memset(mRecords, 0, sizeof(mRecords));
}

PBStorage::~PBStorage()
{
}

void PBStorage::init()
{
    ensureDirectories();
    load();
}

void PBStorage::ensureDirectories()
{
    nn::fs::CreateDirectory(BASE_DIR);
    nn::fs::CreateDirectory(GHOSTS_DIR);
    nn::fs::CreateDirectory(PBS_DIR);

    // Créer le sous-dossier de ghosts pour le profil actuel
    char profileGhostDir[128];
    std::snprintf(profileGhostDir, sizeof(profileGhostDir), "%s/%016llX", GHOSTS_DIR, mProfileId);
    nn::fs::CreateDirectory(profileGhostDir);
}

void PBStorage::setProfileId(u64 profileId)
{
    if (mProfileId != profileId) {
        save(); // Sauvegarder les données de l'ancien profil
        mProfileId = profileId;
        ensureDirectories();
        load(); // Charger les données du nouveau profil
    }
}

int PBStorage::findRecordIndex(KingdomId kingdom, SegmentId segment) const
{
    for (int i = 0; i < mRecordCount; i++) {
        if (mRecords[i].mKingdomId == kingdom && mRecords[i].mSegmentId == segment) {
            return i;
        }
    }
    return -1;
}

const PBRecord* PBStorage::getRecord(KingdomId kingdom, SegmentId segment) const
{
    int idx = findRecordIndex(kingdom, segment);
    if (idx != -1) {
        return &mRecords[idx];
    }
    return nullptr;
}

PBRecord* PBStorage::getOrCreateRecord(KingdomId kingdom, SegmentId segment, const char* name)
{
    int idx = findRecordIndex(kingdom, segment);
    if (idx != -1) {
        return &mRecords[idx];
    }

    if (mRecordCount < MAX_RECORDS) {
        PBRecord& rec = mRecords[mRecordCount++];
        rec.mKingdomId = kingdom;
        rec.mSegmentId = segment;
        if (name) {
            std::strncpy(rec.mSegmentName, name, sizeof(rec.mSegmentName) - 1);
        }
        rec.mBestTicks = 0;
        rec.mBestFrames = 0;
        rec.mHasGhostReplay = false;
        return &rec;
    }

    return nullptr;
}

bool PBStorage::updateRecordIfBetter(KingdomId kingdom, SegmentId segment, const char* name, s64 elapsedTicks, u32 totalFrames, bool& outIsNewPB)
{
    outIsNewPB = false;
    PBRecord* rec = getOrCreateRecord(kingdom, segment, name);
    if (!rec) {
        return false;
    }

    // Si aucun record n'existe encore ou si le temps actuel est inférieur au meilleur temps
    if (rec->mBestTicks == 0 || elapsedTicks < rec->mBestTicks) {
        rec->mBestTicks = elapsedTicks;
        rec->mBestFrames = totalFrames;
        rec->mHasGhostReplay = true;
        if (name && rec->mSegmentName[0] == '\0') {
            std::strncpy(rec->mSegmentName, name, sizeof(rec->mSegmentName) - 1);
        }
        outIsNewPB = true;
        save();
        return true;
    }

    return false;
}

void PBStorage::resetRecord(KingdomId kingdom, SegmentId segment)
{
    int idx = findRecordIndex(kingdom, segment);
    if (idx != -1) {
        mRecords[idx].mBestTicks = 0;
        mRecords[idx].mBestFrames = 0;
        mRecords[idx].mHasGhostReplay = false;

        // Supprimer le fichier fantôme associé s'il existe
        char ghostPath[256];
        getGhostFilePath(kingdom, segment, ghostPath, sizeof(ghostPath));
        if (FsHelper::isFileExist(ghostPath)) {
            nn::fs::DeleteFile(ghostPath);
        }

        save();
    }
}

void PBStorage::getGhostFilePath(KingdomId kingdom, SegmentId segment, char* outBuffer, size_t bufferSize) const
{
    if (!outBuffer || bufferSize == 0) return;
    std::snprintf(outBuffer, bufferSize, "%s/%016llX/k%d_s%d.bin", GHOSTS_DIR, mProfileId, static_cast<int>(kingdom), static_cast<int>(segment));
}

bool PBStorage::save()
{
    char pbsFilePath[256];
    std::snprintf(pbsFilePath, sizeof(pbsFilePath), "%s/pb_%016llX.bin", PBS_DIR, mProfileId);

    struct FileHeader {
        u32 magic;
        u32 count;
        u64 profileId;
    } header = {
        0x53425021, // '!PBS'
        static_cast<u32>(mRecordCount),
        mProfileId
    };

    size_t totalSize = sizeof(FileHeader) + sizeof(PBRecord) * mRecordCount;
    u8* buffer = new u8[totalSize];
    if (!buffer) return false;

    std::memcpy(buffer, &header, sizeof(FileHeader));
    if (mRecordCount > 0) {
        std::memcpy(buffer + sizeof(FileHeader), mRecords, sizeof(PBRecord) * mRecordCount);
    }

    nn::Result res = FsHelper::writeFileToPath(buffer, totalSize, pbsFilePath);
    delete[] buffer;

    return res.IsSuccess();
}

bool PBStorage::load()
{
    char pbsFilePath[256];
    std::snprintf(pbsFilePath, sizeof(pbsFilePath), "%s/pb_%016llX.bin", PBS_DIR, mProfileId);

    if (!FsHelper::isFileExist(pbsFilePath)) {
        mRecordCount = 0;
        return false;
    }

    FsHelper::LoadData data;
    data.path = pbsFilePath;
    FsHelper::loadFileFromPath(data);

    struct FileHeader {
        u32 magic;
        u32 count;
        u64 profileId;
    };

    if (!data.buffer || data.bufSize < static_cast<long>(sizeof(FileHeader))) {
        return false;
    }

    FileHeader* header = reinterpret_cast<FileHeader*>(data.buffer);
    if (header->magic != 0x53425021) {
        delete[] reinterpret_cast<u8*>(data.buffer);
        return false;
    }

    mRecordCount = header->count;
    if (mRecordCount > MAX_RECORDS) {
        mRecordCount = MAX_RECORDS;
    }

    u8* recordsPtr = reinterpret_cast<u8*>(data.buffer) + sizeof(FileHeader);
    std::memcpy(mRecords, recordsPtr, sizeof(PBRecord) * mRecordCount);

    delete[] reinterpret_cast<u8*>(data.buffer);
    return true;
}

} // namespace pe

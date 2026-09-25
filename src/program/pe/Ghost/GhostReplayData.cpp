#include "pe/Ghost/GhostReplayData.h"
#include "helpers/fsHelper.h"
#include <cstring>

namespace pe {

GhostReplayData::GhostReplayData()
{
    mHeader = {};
    mHeader.mMagic = GhostHeader::MAGIC;
    mHeader.mVersion = GhostHeader::CURRENT_VERSION;
}

GhostReplayData::~GhostReplayData()
{
    free();
}

void GhostReplayData::allocate(u32 maxFrames, sead::Heap* heap)
{
    free();
    mHeap = heap;
    mCapacity = maxFrames;
    if (mCapacity > 0) {
        mFrames = new ReplayFrame[mCapacity];
        std::memset(mFrames, 0, sizeof(ReplayFrame) * mCapacity);
    }
}

void GhostReplayData::free()
{
    if (mFrames) {
        delete[] mFrames;
        mFrames = nullptr;
    }
    mCapacity = 0;
    mHeader.mTotalFrames = 0;
}

void GhostReplayData::reset()
{
    mHeader.mTotalFrames = 0;
    mHeader.mTotalTicks = 0;
    if (mFrames && mCapacity > 0) {
        std::memset(mFrames, 0, sizeof(ReplayFrame) * mCapacity);
    }
}

bool GhostReplayData::appendFrame(const ReplayFrame& frame)
{
    if (!mFrames || mCapacity == 0) {
        // Allocation par défaut si non pré-alloué (ex: 36000 frames = 10 minutes à 60 FPS)
        allocate(36000, nullptr);
    }

    if (mHeader.mTotalFrames >= mCapacity) {
        return false; // Buffer plein
    }

    mFrames[mHeader.mTotalFrames] = frame;
    mHeader.mTotalFrames++;
    return true;
}

const ReplayFrame* GhostReplayData::getFrame(u32 index) const
{
    if (!mFrames || index >= mHeader.mTotalFrames) {
        return nullptr;
    }
    return &mFrames[index];
}

bool GhostReplayData::saveToFile(const char* filePath)
{
    if (isEmpty() || !filePath) {
        return false;
    }

    // Calculer la taille totale = En-tête + Toutes les frames enregistrées
    size_t dataSize = sizeof(GhostHeader) + (sizeof(ReplayFrame) * mHeader.mTotalFrames);
    
    // Créer un buffer temporaire contigu
    u8* tempBuffer = new u8[dataSize];
    if (!tempBuffer) {
        return false;
    }

    std::memcpy(tempBuffer, &mHeader, sizeof(GhostHeader));
    std::memcpy(tempBuffer + sizeof(GhostHeader), mFrames, sizeof(ReplayFrame) * mHeader.mTotalFrames);

    nn::Result res = FsHelper::writeFileToPath(tempBuffer, dataSize, filePath);
    delete[] tempBuffer;

    return res.IsSuccess();
}

bool GhostReplayData::loadFromFile(const char* filePath, sead::Heap* heap)
{
    if (!filePath || !FsHelper::isFileExist(filePath)) {
        return false;
    }

    long fileSize = FsHelper::getFileSize(filePath);
    if (fileSize <= static_cast<long>(sizeof(GhostHeader))) {
        return false;
    }

    FsHelper::LoadData loadData;
    loadData.path = filePath;
    FsHelper::loadFileFromPath(loadData);

    if (!loadData.buffer || loadData.bufSize < static_cast<long>(sizeof(GhostHeader))) {
        return false;
    }

    GhostHeader* headerPtr = reinterpret_cast<GhostHeader*>(loadData.buffer);
    if (headerPtr->mMagic != GhostHeader::MAGIC || headerPtr->mVersion != GhostHeader::CURRENT_VERSION) {
        delete[] reinterpret_cast<u8*>(loadData.buffer);
        return false;
    }

    mHeader = *headerPtr;
    u32 frameCount = mHeader.mTotalFrames;

    // Allouer le buffer mémoire pour stocker les frames du replay
    allocate(frameCount, heap);
    mHeader.mTotalFrames = frameCount;

    u8* framesPtr = reinterpret_cast<u8*>(loadData.buffer) + sizeof(GhostHeader);
    std::memcpy(mFrames, framesPtr, sizeof(ReplayFrame) * frameCount);

    delete[] reinterpret_cast<u8*>(loadData.buffer);
    return true;
}

} // namespace pe

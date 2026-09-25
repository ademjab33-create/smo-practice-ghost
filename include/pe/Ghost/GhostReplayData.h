#pragma once

#include "basis/seadTypes.h"
#include "pe/Ghost/GhostTypes.h"
#include <sead/heap/seadHeap.h>

namespace pe {

class GhostReplayData {
public:
    GhostReplayData();
    ~GhostReplayData();

    void allocate(u32 maxFrames, sead::Heap* heap);
    void free();
    void reset();

    // Enregistrement
    bool appendFrame(const ReplayFrame& frame);
    
    // Lecture
    const ReplayFrame* getFrame(u32 index) const;
    u32 getFrameCount() const { return mHeader.mTotalFrames; }
    const GhostHeader& getHeader() const { return mHeader; }
    GhostHeader& getHeader() { return mHeader; }

    // Sauvegarde & Chargement SD
    bool saveToFile(const char* filePath);
    bool loadFromFile(const char* filePath, sead::Heap* heap);

    bool isEmpty() const { return mHeader.mTotalFrames == 0 || mFrames == nullptr; }

private:
    GhostHeader mHeader;
    ReplayFrame* mFrames = nullptr;
    u32 mCapacity = 0;
    sead::Heap* mHeap = nullptr;
};

} // namespace pe

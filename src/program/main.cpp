#include "al/Library/Memory/MemorySystem.h"
#include "al/Library/Sequence/Sequence.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include "diag/assert.hpp"
#include "heap/seadExpHeap.h"
#include "heap/seadHeapMgr.h"
#include "hook/trampoline.hpp"
#include "lib.hpp"
#include "nn/fs.h"
#include "pe/Util/Offsets.h"
#include "program/imgui_nvn.h"
#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include "pe/Ghost/PBOverlay.h"
#include <sead/filedevice/seadFileDeviceMgr.h>

static sead::ExpHeap* sGhostHeap = nullptr;

HOOK_DEFINE_TRAMPOLINE(FileDeviceMgrCtor) { static void Callback(sead::FileDeviceMgr * thisPtr); };
void FileDeviceMgrCtor::Callback(sead::FileDeviceMgr* thisPtr) { Orig(thisPtr); thisPtr->mMountedSd = nn::fs::MountSdCardForDebug("sd").IsSuccess(); }

class HakoniwaSequence;

HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceInit) { static void Callback(HakoniwaSequence * thisPtr, const al::SequenceInitInfo& info); };
void HakoniwaSequenceInit::Callback(HakoniwaSequence* thisPtr, const al::SequenceInitInfo& info) {
    Orig(thisPtr, info);
    sGhostHeap = sead::ExpHeap::create(1024 * 1024 * 4, "GhostHeap", al::getSequenceHeap(), 8, sead::ExpHeap::cHeapDirection_Forward, false);
    sead::ScopedCurrentHeapSetter setter(sGhostHeap);
    pe::PBStorage::createInstance(nullptr);
    pe::PBStorage::instance()->init();
    pe::ILTracker::createInstance(nullptr);
    pe::ILTracker::instance()->init();
    pe::GhostManager::createInstance(nullptr);
}

HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceUpdate) { static void Callback(HakoniwaSequence * thisPtr); };
void HakoniwaSequenceUpdate::Callback(HakoniwaSequence* thisPtr) {
    Orig(thisPtr);
    if (!sGhostHeap) return;
    sead::ScopedCurrentHeapSetter setter(sGhostHeap);
    al::Scene* scene = reinterpret_cast<al::Sequence*>(thisPtr)->getCurrentScene();
    auto* ghostMgr = pe::GhostManager::instance();
    if (ghostMgr) {
        ghostMgr->update(scene);
        if (al::isPadHoldR(-1)) {
            if (al::isPadTriggerUp(-1))   ghostMgr->startRun();
            if (al::isPadTriggerDown(-1)) ghostMgr->stopRun();
            if (al::isPadTriggerLeft(-1)) ghostMgr->resetRun();
        }
    }
}

HOOK_DEFINE_TRAMPOLINE(SceneEndInitHook) {
    static void Callback(al::Scene* scene, const al::ActorInitInfo& info) {
        Orig(scene, info);
        if (!sGhostHeap) return;
        sead::ScopedCurrentHeapSetter setter(sGhostHeap);
        auto* ghostMgr = pe::GhostManager::instance();
        if (ghostMgr) ghostMgr->initPuppet(info);
    }
};

static void drawDbgGui() {
    if (!sGhostHeap) return;
    sead::ScopedCurrentHeapSetter setter(sGhostHeap);
    pe::PBOverlay::draw();
}

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();
    FileDeviceMgrCtor::InstallAtOffset(pe::offsets::FileDeviceMgrCtorHookLocation);
    HakoniwaSequenceInit::InstallAtOffset(pe::offsets::HakoniwaSequenceInitHookLocation);
    HakoniwaSequenceUpdate::InstallAtOffset(pe::offsets::HakoniwaSequenceUpdate);
    SceneEndInitHook::InstallAtOffset(pe::offsets::SceneEndInitHookLocation);
    nvnImGui::InstallHooks();
    nvnImGui::addDrawFunc(drawDbgGui);
}

extern "C" NORETURN void exl_exception_entry() { EXL_ABORT(0x420); }
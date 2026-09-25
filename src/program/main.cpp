#include "al/Library/Memory/MemorySystem.h"
#include "al/Library/Sequence/Sequence.h"
#include "diag/assert.hpp"
#include "heap/seadExpHeap.h"
#include "heap/seadHeapMgr.h"
#include "hook/trampoline.hpp"
#include "lib.hpp"
#include "nn/fs.h"
#include "pe/Hacks/MenuDPadDisable.h"
#include "pe/Hacks/PracticeHacks.h"
#include "pe/Menu/Menu.h"
#include "pe/Menu/UserConfig.h"
#include "pe/Util/Offsets.h"
#include "program/imgui_nvn.h"
#include "pe/Ghost/GhostManager.h"
#include "pe/Ghost/ILTracker.h"
#include "pe/Ghost/PBStorage.h"
#include <sead/filedevice/seadFileDeviceMgr.h>

HOOK_DEFINE_TRAMPOLINE(FileDeviceMgrCtor) { static void Callback(sead::FileDeviceMgr * thisPtr); };
void FileDeviceMgrCtor::Callback(sead::FileDeviceMgr* thisPtr)
{
    Orig(thisPtr);
    thisPtr->mMountedSd = nn::fs::MountSdCardForDebug("sd").IsSuccess();
}

class HakoniwaSequence;
HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceInit) { static void Callback(HakoniwaSequence * thisPtr, const al::SequenceInitInfo& info); };
void HakoniwaSequenceInit::Callback(HakoniwaSequence* thisPtr, const al::SequenceInitInfo& info)
{
    Orig(thisPtr, info);

    pe::getMenuHeap() = sead::ExpHeap::create(1024 * 1024 * 8, "MenuHeap", al::getSequenceHeap(), 8, sead::ExpHeap::cHeapDirection_Forward, false);

    sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
    pe::PBStorage::createInstance(nullptr);
    pe::PBStorage::instance()->init();
    pe::ILTracker::createInstance(nullptr);
    pe::ILTracker::instance()->init();
    pe::GhostManager::createInstance(nullptr);
    pe::Menu::createInstance(nullptr);
}

HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceUpdate) { static void Callback(HakoniwaSequence * thisPtr); };
void HakoniwaSequenceUpdate::Callback(HakoniwaSequence* thisPtr)
{
    Orig(thisPtr);

    if (!pe::getMenuHeap()) return;
    sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
    auto* menu = pe::Menu::instance();
    if (menu) {
        al::Scene* scene = reinterpret_cast<al::Sequence*>(thisPtr)->getCurrentScene();
        menu->update(scene);
    }
}

HOOK_DEFINE_TRAMPOLINE(SceneEndInitHook) {
    static void Callback(al::Scene* scene, const al::ActorInitInfo& info) {
        Orig(scene, info);
        if (!pe::getMenuHeap()) return;
        sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
        auto* ghostMgr = pe::GhostManager::instance();
        if (ghostMgr) {
            ghostMgr->initPuppet(info);
        }
    }
};

static void drawDbgGui()
{
    if (!pe::getMenuHeap()) return;
    sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
    auto* menu = pe::Menu::instance();
    if (menu)
        menu->draw();
}

extern "C" void exl_main(void* x0, void* x1)
{
    exl::hook::Initialize();

    if (!pe::getConfig()) {
        pe::getConfig() = new pe::UserConfig();
    }

    FileDeviceMgrCtor::InstallAtOffset(pe::offsets::FileDeviceMgrCtorHookLocation);
    HakoniwaSequenceInit::InstallAtOffset(pe::offsets::HakoniwaSequenceInitHookLocation);
    HakoniwaSequenceUpdate::InstallAtOffset(pe::offsets::HakoniwaSequenceUpdate);
    SceneEndInitHook::InstallAtOffset(pe::offsets::SceneEndInitHookLocation);

    pe::initMenuDPadDisableHooks();
    pe::installPracticeHacks();

    nvnImGui::InstallHooks();
    nvnImGui::addDrawFunc(drawDbgGui);
}

extern "C" NORETURN void exl_exception_entry()
{
    EXL_ABORT(0x420);
}
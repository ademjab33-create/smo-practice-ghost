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
#include <sead/filedevice/seadFileDeviceMgr.h>

HOOK_DEFINE_TRAMPOLINE(FileDeviceMgrCtor) { static void Callback(sead::FileDeviceMgr * thisPtr); };
void FileDeviceMgrCtor::Callback(sead::FileDeviceMgr* thisPtr)
{
    Orig(thisPtr);
    thisPtr->mMountedSd = nn::fs::MountSdCardForDebug("sd").IsSuccess();
}

namespace al {
sead::Heap* getSequenceHeap() {
    using FuncType = sead::Heap* (*)();
    static FuncType sFunc = reinterpret_cast<FuncType>(exl::util::modules::GetTargetOffset(0x006a2990));
    return sFunc();
}
}

class HakoniwaSequence;
static HakoniwaSequence* sHakoniwaSequence = nullptr;

HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceInit) { static void Callback(HakoniwaSequence * thisPtr, const al::SequenceInitInfo& info); };
void HakoniwaSequenceInit::Callback(HakoniwaSequence* thisPtr, const al::SequenceInitInfo& info)
{
    Orig(thisPtr, info);

    sHakoniwaSequence = thisPtr;

    auto* seqHeap = al::getSequenceHeap();
    pe::getMenuHeap() = sead::ExpHeap::create(1024 * 512, "MenuHeap", seqHeap, 8, sead::ExpHeap::cHeapDirection_Forward, false);
    if (!pe::getMenuHeap()) {
        pe::getMenuHeap() = seqHeap;
    }

    sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
    pe::Menu::createInstance(nullptr);
}

static void drawDbgGui()
{
    sead::ScopedCurrentHeapSetter setter(pe::getMenuHeap());
    auto* menu = pe::Menu::instance();
    if (menu) {
        al::Scene* scene = nullptr;
        if (sHakoniwaSequence) {
            scene = *reinterpret_cast<al::Scene**>(reinterpret_cast<u8*>(sHakoniwaSequence) + 0xb0);
        }
        menu->update(scene);
        menu->draw();
    }
}

extern "C" void exl_main(void* x0, void* x1)
{
    exl::hook::Initialize();

    FileDeviceMgrCtor::InstallAtOffset(pe::offsets::FileDeviceMgrCtorHookLocation);
    HakoniwaSequenceInit::InstallAtOffset(pe::offsets::HakoniwaSequenceInitHookLocation);

    pe::initMenuDPadDisableHooks();
    pe::installPracticeHacks();

    nvnImGui::InstallHooks();
    nvnImGui::addDrawFunc(drawDbgGui);
}

extern "C" NORETURN void exl_exception_entry()
{
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}

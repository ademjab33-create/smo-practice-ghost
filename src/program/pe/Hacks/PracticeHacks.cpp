#include "pe/Hacks/PracticeHacks.h"
#include "Layout/MapLayout.h"
#include "Scene/StageScene.h"
#include "Scene/StageSceneStateStageMap.h"
#include "System/GameDataHolderWriter.h"
#include "al/Library/Audio/AudioKeeper.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Math/MathRandomUtil.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "hook/trampoline.hpp"
#include "patch/code_patcher.hpp"
#include "pe/Menu/Menu.h"
#include "pe/Menu/UserConfig.h"
#include "pe/Util/Offsets.h"
#include "Player/PlayerActorHakoniwa.h"
#include "replace.hpp"
#include "util/modules.hpp"
#include "util/sys/rw_pages.hpp"
#include <cmath>

namespace pe {

HOOK_DEFINE_TRAMPOLINE(IsGotShine) { static bool Callback(void* accessor, void* shineInfo); };
HOOK_DEFINE_TRAMPOLINE(SetGotShine) { static void Callback(GameDataHolderWriter writer, void* shineInfo); };
HOOK_DEFINE_TRAMPOLINE(StartBgm1) { static void Callback(al::IUseAudioKeeper * user, const char* bgm, int a, int b); };
HOOK_DEFINE_TRAMPOLINE(StartBgm2) { static void Callback(al::IUseAudioKeeper * user, void* request, bool a, bool b); };
HOOK_DEFINE_TRAMPOLINE(IsPadTriggerA) { static bool Callback(s32 port); };
HOOK_DEFINE_TRAMPOLINE(DoCheckpointTouchNotify) { static void Callback(al::LiveActor * checkpoint); };

// Hooks from Practice 1.0 (BTT-Studio)
HOOK_DEFINE_TRAMPOLINE(NoDamageHook) { static void Callback(GameDataHolderWriter writer); };
HOOK_DEFINE_TRAMPOLINE(RefreshPurpsHook) { static bool Callback(void* accessor, const void* info); };
HOOK_DEFINE_TRAMPOLINE(DoorRefreshHook) { static void Callback(al::LiveActor* door, const void* info); };
HOOK_DEFINE_TRAMPOLINE(SeedGrowTimeHook) { static int Callback(const al::LiveActor* actor, const void* id); };
HOOK_DEFINE_TRAMPOLINE(SeedUsedHook) { static bool Callback(const al::LiveActor* actor, const void* id); };
HOOK_DEFINE_TRAMPOLINE(KingdomEnterHook) { static bool Callback(void* data, int i); };
HOOK_DEFINE_TRAMPOLINE(DisableMoonLockHook) { static int Callback(void* file, void* a, void* b, int worldID); };
HOOK_DEFINE_TRAMPOLINE(AllCheckpointsHook) { static bool Callback(void* file, int checkpointIdx); };
HOOK_DEFINE_TRAMPOLINE(CloudSkipHook) { static bool Callback(StageScene* stageScene); };
HOOK_DEFINE_TRAMPOLINE(SkipBroodalsHook) { static bool Callback(StageScene* stageScene); };
HOOK_DEFINE_TRAMPOLINE(HintPhotoHook) { static bool Callback(const al::LiveActor* actor, const char* name); };
HOOK_DEFINE_TRAMPOLINE(ToadRefreshHook) { static bool Callback(const al::LiveActor* actor); };
HOOK_DEFINE_TRAMPOLINE(NoclipHook) { static void Callback(PlayerActorHakoniwa* player); };
HOOK_DEFINE_TRAMPOLINE(WorldWarpChangeStageHook) { static void Callback(GameDataHolder* thisPtr, const char* stageName); };

// Cutscene skip hooks
HOOK_DEFINE_TRAMPOLINE(RsDemoHook) { static bool Callback(const al::Scene* scene); };
HOOK_DEFINE_TRAMPOLINE(FirstDemoScenarioHook) { static bool Callback(const al::LiveActor* actor); };
HOOK_DEFINE_TRAMPOLINE(FirstDemoWorldHook) { static bool Callback(const al::Scene* scene); };
HOOK_DEFINE_TRAMPOLINE(FirstDemoMoonRockHook) { static bool Callback(void* thisPtr); };
HOOK_DEFINE_TRAMPOLINE(ShowDemoHackHook) { static bool Callback(void* thisPtr); };

static const char* sCustomMoonNames[] = {
    "Practice Mod", "Hi", "Practice", "Hi BTT", "BTT", "I'm cool", "Super Mario Odyssey", " "
};

bool IsGotShine::Callback(void* accessor, void* shineInfo)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsGrayShineRefreshEnabled) ? false : Orig(accessor, shineInfo);
}

void SetGotShine::Callback(GameDataHolderWriter writer, void* shineInfo)
{
    auto* cfg = getConfig();
    if (!cfg || !cfg->mIsMoonRefreshEnabled)
        Orig(writer, shineInfo);
}

void StartBgm1::Callback(al::IUseAudioKeeper* user, const char* bgm, int a, int b)
{
    auto* cfg = getConfig();
    if (!cfg || !cfg->mIsBgmDisabled)
        Orig(user, bgm, a, b);
}

void StartBgm2::Callback(al::IUseAudioKeeper* user, void* request, bool a, bool b)
{
    auto* cfg = getConfig();
    if (!cfg || !cfg->mIsBgmDisabled)
        Orig(user, request, a, b);
}

static bool isPatternReverse()
{
    bool b = al::isHalfProbability();
    auto* cfg = getConfig();
    if (cfg && cfg->mCurPattern != MofumofuPattern::Random)
        b = UserConfig::sPatternEntries[cfg->mCurPattern - 1].reverse;
    return b;
}

static int getMofumofuTarget(int a)
{
    int r = al::getRandom(a);
    auto* cfg = getConfig();
    if (cfg && cfg->mCurPattern != MofumofuPattern::Random)
        r = UserConfig::sPatternEntries[cfg->mCurPattern - 1].target;
    return r;
}

static bool isEnableSave(StageScene* scene)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsEnableAutosave) ? scene->isEnableSave() : false;
}

static bool isEnableCheckpointWarp(MapLayout* thisPtr)
{
    auto* cfg = getConfig();
    return (cfg && (cfg->mIsAlwaysCheckpoints || cfg->mIsAllCheckpointsEnabled)) ? true : thisPtr->isEnableCheckpointWarp();
}

bool IsPadTriggerA::Callback(s32 port)
{
    auto* menu = pe::Menu::instance();
    if (menu && menu->isEnabled())
        return false;
    return Orig(port);
}

static void setMapTargetUpdateNullNerve(al::IUseNerve* user, const al::Nerve* nerve)
{
    StageSceneStateStageMap* map;
    __asm("mov %0, x19"
          : "=r"(map));
    void* mapThingPtr;
    __asm("mov %0, x23"
          : "=r"(mapThingPtr));

    if (mapThingPtr != nullptr && Menu::instance())
        Menu::instance()->setLatestMapTarget(mapThingPtr);

    al::setNerve(user, nerve);
}

void DoCheckpointTouchNotify::Callback(al::LiveActor* checkpoint)
{
    auto* cfg = getConfig();
    if (!cfg || !cfg->mDisableCheckpointTouching)
        Orig(checkpoint);
}

void NoDamageHook::Callback(GameDataHolderWriter writer)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsNoDamageEnabled) return;
    Orig(writer);
}

static bool warpTextHook(void* accessor)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsWarpTextRefreshEnabled) return false;
    if (!accessor) return false;
    void* file = *(void**)((uintptr_t)accessor + 0x20);
    if (!file) return false;
    return *(uint8_t*)((uintptr_t)file + 0xab8) != 0;
}

static bool shardRefreshHook(void* a, void* b)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsMoonShardsRefreshEnabled) return false;
    typedef bool (*OrigFn)(void*, void*);
    static OrigFn orig = (OrigFn)exl::util::modules::GetTargetOffset(0x0058d210);
    return orig ? orig(a, b) : false;
}

bool RefreshPurpsHook::Callback(void* accessor, const void* info)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsPurpleCoinsRefreshEnabled) ? false : Orig(accessor, info);
}

void DoorRefreshHook::Callback(al::LiveActor* door, const void* info)
{
    Orig(door, info);
    auto* cfg = getConfig();
    if (cfg && cfg->mIsDoorRefreshEnabled) {
        typedef void (*SwitchCloseAgainFn)(al::LiveActor*);
        static SwitchCloseAgainFn sSwitchCloseAgain = (SwitchCloseAgainFn)exl::util::modules::GetTargetOffset(0x002301c0);
        if (sSwitchCloseAgain) sSwitchCloseAgain(door);
    }
}

int SeedGrowTimeHook::Callback(const al::LiveActor* actor, const void* id)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsSeedsGrowing) return 999999;
    if (cfg && cfg->mIsSeedsRefreshEnabled) return 0;
    return Orig(actor, id);
}

bool SeedUsedHook::Callback(const al::LiveActor* actor, const void* id)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsSeedsRefreshEnabled) return false;
    return Orig(actor, id);
}

bool KingdomEnterHook::Callback(void* data, int i)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsKingdomEnterCutsceneRefreshEnabled) ? false : Orig(data, i);
}

int DisableMoonLockHook::Callback(void* file, void* a, void* b, int worldID)
{
    int lockSize = Orig(file, a, b, worldID);
    auto* cfg = getConfig();
    return (cfg && cfg->mIsDisableMoonLock) ? 0 : lockSize;
}

bool AllCheckpointsHook::Callback(void* file, int checkpointIdx)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsAllCheckpointsEnabled) return true;
    return Orig(file, checkpointIdx);
}

bool CloudSkipHook::Callback(StageScene* stageScene)
{
    static int functionCalls = 0;
    auto* cfg = getConfig();
    if (!cfg || !cfg->mIsSkipCloudEnabled) {
        functionCalls = 0;
        return Orig(stageScene);
    }
    functionCalls++;
    if (functionCalls == 2) {
        functionCalls = 0;
        if (cfg && cfg->mTimerAutoKingdom && Timer::sInstance) {
            Timer::sInstance->stop();
        }
        return true;
    }
    return false;
}

bool SkipBroodalsHook::Callback(StageScene* stageScene)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsSkipBroodalsEnabled) return true;
    return Orig(stageScene);
}

bool HintPhotoHook::Callback(const al::LiveActor* actor, const char* name)
{
    auto* cfg = getConfig();
    if (cfg && (cfg->mIsHintPhotoSpawnEnabled || cfg->mIsLuigiPixelsEnabled)) return true;
    return Orig(actor, name);
}

bool ToadRefreshHook::Callback(const al::LiveActor* actor)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsCutsceneRefreshEnabled) ? false : Orig(actor);
}

bool RsDemoHook::Callback(const al::Scene* scene)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsAlwaysManualCutscene) ? true : Orig(scene);
}

bool FirstDemoScenarioHook::Callback(const al::LiveActor* actor)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsAlwaysManualCutscene) ? true : Orig(actor);
}

bool FirstDemoWorldHook::Callback(const al::Scene* scene)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsAlwaysManualCutscene) ? true : Orig(scene);
}

bool FirstDemoMoonRockHook::Callback(void* thisPtr)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsAlwaysManualCutscene) ? true : Orig(thisPtr);
}

bool ShowDemoHackHook::Callback(void* thisPtr)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsAlwaysManualCutscene) ? true : Orig(thisPtr);
}

void WorldWarpChangeStageHook::Callback(GameDataHolder* thisPtr, const char* stageName)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mTimerAutoKingdom && Timer::sInstance) {
        Timer::sInstance->stop();
    }
    Orig(thisPtr, stageName);
}

void NoclipHook::Callback(PlayerActorHakoniwa* player)
{
    static bool wasNoclipOn = false;
    bool isNoclip = (getConfig() && getConfig()->mIsNoclipEnabled);
    if (!isNoclip && wasNoclipOn)
        player->endDemoPuppetable();
    wasNoclipOn = isNoclip;

    if (!isNoclip) {
        Orig(player);
        return;
    }

    player->startDemoPuppetable();

    sead::Vector3f playerPos = al::getTrans(player);

    const sead::Vector2f& leftStick = al::getLeftStick(-1);
    const sead::Vector2f& rightStick = al::getRightStick(-1);

    float speed = 25.0f;
    if (al::isPadHoldX(-1) || al::isPadHoldY(-1)) speed = 75.0f;

    playerPos.x += leftStick.x * speed;
    playerPos.z -= leftStick.y * speed;

    if (al::isPadHoldZL(-1) || al::isPadHoldL(-1)) playerPos.y -= speed;
    if (al::isPadHoldZR(-1) || al::isPadHoldR(-1)) playerPos.y += speed;
    if (std::abs(rightStick.y) > 0.1f) playerPos.y += rightStick.y * speed;

    al::setTrans(player, playerPos);

    Orig(player);
}

void installPracticeHacks()
{
    using Patcher = exl::patch::CodePatcher;

    IsGotShine::InstallAtOffset(offsets::GameDataFileIsGotShine);
    SetGotShine::InstallAtOffset(offsets::GameDataFunctionSetGotShine);
    StartBgm1::InstallAtOffset(offsets::StartBgm1);
    StartBgm2::InstallAtOffset(offsets::StartBgm2);
    DoCheckpointTouchNotify::InstallAtOffset(offsets::CheckpointTouchHook);

    Patcher(0x000a46ec).BranchLinkInst((void*)getMofumofuTarget);
    Patcher(0x000a4698).BranchLinkInst((void*)isPatternReverse);
    Patcher(0x001d1584).BranchLinkInst((void*)isEnableCheckpointWarp);
    Patcher(0x004742d0).BranchLinkInst((void*)isEnableSave);
    Patcher(0x004b1c78).BranchLinkInst((void*)isEnableSave);
    Patcher(0x004b4fa4).BranchLinkInst((void*)isEnableSave);

    Patcher(0x0049d3d0).BranchLinkInst((void*)setMapTargetUpdateNullNerve);
    IsPadTriggerA::InstallAtOffset(0x005cfbd0);

    // Practice 1.0 options
    RefreshPurpsHook::InstallAtOffset(0x004d4da0);
    DoorRefreshHook::InstallAtOffset(0x0022ff40);
    Patcher(0x001b87ac).BranchLinkInst((void*)shardRefreshHook);
    KingdomEnterHook::InstallAtOffset(0x004e1060);
    Patcher(0x004d3880).BranchInst((void*)warpTextHook);
    NoDamageHook::InstallAtOffset(0x004d3a30);
    SeedGrowTimeHook::InstallAtOffset(0x004dd230);
    SeedUsedHook::InstallAtOffset(0x004dd260);
    CloudSkipHook::InstallAtOffset(0x004b3c80);
    AllCheckpointsHook::InstallAtOffset(0x004cf720);
    DisableMoonLockHook::InstallAtOffset(0x004cda80);
    ToadRefreshHook::InstallAtOffset(0x004dd520);

    exl::util::RwPages a(exl::util::modules::GetTargetOffset(offsets::ShineRefreshText), 24);
    strncpy((char*)a.GetRw(), "Practice Mod", 24);
}

} // namespace pe

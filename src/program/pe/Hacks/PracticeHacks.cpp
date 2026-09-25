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
#include "pe/Ghost/ILTracker.h"
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
HOOK_DEFINE_TRAMPOLINE(WarpTextHook) { static bool Callback(void* accessor); };
HOOK_DEFINE_TRAMPOLINE(RefreshPurpsHook) { static bool Callback(void* accessor, const void* info); };
HOOK_DEFINE_TRAMPOLINE(DoorRefreshHook) { static void Callback(al::LiveActor* door, const void* info); };
HOOK_DEFINE_TRAMPOLINE(SeedGrowTimeHook) { static int Callback(const al::LiveActor* actor, const void* id); };
HOOK_DEFINE_TRAMPOLINE(SeedUsedHook) { static bool Callback(const al::LiveActor* actor, const void* id); };
HOOK_DEFINE_TRAMPOLINE(KingdomEnterHook) { static bool Callback(void* data, int i); };
HOOK_DEFINE_TRAMPOLINE(DisableMoonLockHook) { static int Callback(GameDataHolder* thisPtr, bool* isCrashList, int worldID); };
HOOK_DEFINE_TRAMPOLINE(AllCheckpointsHook) { static bool Callback(void* acc, int checkpointIdx); };
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
    if (cfg && cfg->mMoonNameIdx >= 0 && cfg->mMoonNameIdx < 8) {
        exl::util::RwPages a(exl::util::modules::GetTargetOffset(offsets::ShineRefreshText), 24);
        strncpy((char*)a.GetRw(), sCustomMoonNames[cfg->mMoonNameIdx], 24);
    }
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

static bool isEnableCheckpointWarp(MapLayout* thisPtr)
{
    auto* cfg = getConfig();
    return (cfg && (cfg->mIsAlwaysCheckpoints || cfg->mIsAllCheckpointsEnabled)) ? true : thisPtr->isEnableCheckpointWarp();
}

static bool isEnableSave(StageScene* scene)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsEnableAutosave) ? scene->isEnableSave() : false;
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

bool WarpTextHook::Callback(void* accessor)
{
    auto* cfg = getConfig();
    return (cfg && cfg->mIsWarpTextRefreshEnabled) ? false : Orig(accessor);
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
        static void (*sSwitchCloseAgain)(al::LiveActor*) = nullptr;
        if (!sSwitchCloseAgain) {
            nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sSwitchCloseAgain), "_ZN14DoorAreaChange16switchCloseAgainEv");
        }
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

int DisableMoonLockHook::Callback(GameDataHolder* thisPtr, bool* isCrashList, int worldID)
{
    int lockSize = Orig(thisPtr, isCrashList, worldID);
    auto* cfg = getConfig();
    return (cfg && cfg->mIsDisableMoonLock) ? 0 : lockSize;
}

bool AllCheckpointsHook::Callback(void* acc, int checkpointIdx)
{
    auto* cfg = getConfig();
    if (cfg && cfg->mIsAllCheckpointsEnabled) return true;
    return Orig(acc, checkpointIdx);
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
        if (ILTracker::instance()) {
            ILTracker::instance()->endRun();
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
    if (ILTracker::instance()) {
        if (ILTracker::instance()->getCurrentKingdom() == KingdomId::Cap) {
            ILTracker::instance()->notifyCapEnd();
        } else {
            ILTracker::instance()->notifyOdyssey();
        }
        ILTracker::instance()->notifyBlackScreen();
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

    sead::Vector3f* playerPos = al::getTransPtr(player);
    if (!playerPos) {
        Orig(player);
        return;
    }

    const sead::Vector2f& leftStick = al::getLeftStick(-1);
    const sead::Vector2f& rightStick = al::getRightStick(-1);

    float speed = 25.0f;
    if (al::isPadHoldX(-1) || al::isPadHoldY(-1)) speed = 75.0f;

    sead::Vector3f front = al::getFront(player);
    sead::Vector3f right(front.z, 0.0f, -front.x);

    playerPos->x += (front.x * leftStick.y + right.x * leftStick.x) * speed;
    playerPos->z += (front.z * leftStick.y + right.z * leftStick.x) * speed;

    if (al::isPadHoldZL(-1) || al::isPadHoldL(-1)) playerPos->y -= speed;
    if (al::isPadHoldZR(-1) || al::isPadHoldR(-1)) playerPos->y += speed;
    if (std::abs(rightStick.y) > 0.1f) playerPos->y += rightStick.y * speed;

    Orig(player);
}

void installPracticeHacks()
{
#if GAME_VERSION == 130
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

    exl::util::RwPages a(exl::util::modules::GetTargetOffset(offsets::ShineRefreshText), 24);
    strncpy((char*)a.GetRw(), "Practice Mod", 24);
#endif

    NoDamageHook::InstallAtSymbol("_ZN16GameDataFunction12damagePlayerE20GameDataHolderWriter");
    WarpTextHook::InstallAtSymbol("_ZN16GameDataFunction34isAlreadyShowExplainCheckpointFlagE22GameDataHolderAccessor");
    RefreshPurpsHook::InstallAtSymbol("_ZN16GameDataFunction16isGotCoinCollectE22GameDataHolderAccessorRKN2al13ActorInitInfoE");
    DoorRefreshHook::InstallAtSymbol("_ZN14DoorAreaChange4initERKN2al13ActorInitInfoE");
    SeedGrowTimeHook::InstallAtSymbol("_ZN2rs17getGrowFlowerTimeEPKN2al9LiveActorEPKNS0_11PlacementIdE");
    SeedUsedHook::InstallAtSymbol("_ZN2rs20isUsedGrowFlowerSeedEPKN2al9LiveActorEPKNS0_11PlacementIdE");
    KingdomEnterHook::InstallAtSymbol("_ZNK16GameProgressData16isAlreadyGoWorldEi");
    DisableMoonLockHook::InstallAtSymbol("_ZNK14GameDataHolder18findUnlockShineNumEPbi");
    AllCheckpointsHook::InstallAtSymbol("_ZN16GameDataFunction22isGotCheckpointInWorldE22GameDataHolderAccessori");
    CloudSkipHook::InstallAtSymbol("_ZNK10StageScene16isDefeatKoopaLv1Ev");
    SkipBroodalsHook::InstallAtSymbol("_ZNK10StageScene14isDefeatBossLv1Ev");
    HintPhotoHook::InstallAtSymbol("_ZN2rs19checkSavedHintPhotoEPKN2al9LiveActorEPKc");
    ToadRefreshHook::InstallAtSymbol("_ZN2rs34isOnFlagKinopioBrigadeNpcFirstTalkEPKN2al9LiveActorE");
    WorldWarpChangeStageHook::InstallAtSymbol("_ZN14GameDataHolder31changeNextStageWithDemoWorldWarpEPKc");
    NoclipHook::InstallAtSymbol("_ZN19PlayerActorHakoniwa8movementEv");

    RsDemoHook::InstallAtSymbol("_ZN2rs11isFirstDemoEPKN2al5SceneE");
    FirstDemoScenarioHook::InstallAtSymbol("_ZN2rs30isFirstDemoScenarioStartCameraEPKN2al9LiveActorE");
    FirstDemoWorldHook::InstallAtSymbol("_ZN2rs27isFirstDemoWorldIntroCameraEPKN2al5SceneE");
    FirstDemoMoonRockHook::InstallAtSymbol("_ZNK12MoonRockData38isEnableShowDemoAfterOpenMoonRockFirstEv");
    ShowDemoHackHook::InstallAtSymbol("_ZNK18DemoStateHackFirst20isEnableShowHackDemoEv");
}

} // namespace pe

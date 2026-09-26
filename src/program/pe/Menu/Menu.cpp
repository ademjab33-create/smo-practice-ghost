#include "pe/Menu/Menu.h"
#include "MapObj/Shine.h"
#include "MapObj/TreasureBox.h"
#include "Player/PlayerActorHakoniwa.h"
#include "Player/PlayerDamageKeeper.h"
#include "Scene/ChangeStageInfo.h"
#include "Scene/SceneObjUtil.h"
#include "Scene/StageScene.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderWriter.h"
#include "System/GameDataHolderAccessor.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include "al/Library/LiveActor/ActorCollisionFunction.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/LiveActor/LiveActorGroup.h"
#include "al/Library/LiveActor/LiveActorKit.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "al/Library/Scene/Scene.h"
#include "al/Library/Thread/FunctorV0M.h"
#include "imgui.h"
#include "pe/Menu/Action.h"
#include "pe/Menu/BoolMenuComponent.h"
#include "pe/Menu/ButtonMenuComponent.h"
#include "pe/Menu/ChangeStage.h"
#include "pe/Menu/DummyMenuComponent.h"
#include "pe/Menu/EnumMenuComponent.h"
#include "pe/Menu/InputDisplay.h"
#include "pe/Menu/IntMenuComponent.h"
#include "pe/Menu/MenuComponent.h"
#include "pe/Menu/MofumofuPatternUpdateNotification.h"
#include "pe/Menu/QuickActionMenu.h"
#include "pe/Menu/Timer.h"
#include "pe/Menu/UserConfig.h"
#include "pe/Menu/Vector2MenuComponent.h"
#include "pe/Util/Localization.h"
#include "pe/Util/Nerve.h"
#include "pe/Util/Offsets.h"
#include "pe/Util/Type.h"
#include "rs/Util/PlayerUtil.h"
#include "util/modules.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>

namespace pe {

sead::Heap*& getMenuHeap()
{
    static sead::Heap* sHeap { nullptr };
    return sHeap;
}

SEAD_SINGLETON_DISPOSER_IMPL(Menu)

static constexpr const char* sPatternNames[] {
    "Random", "Ghost", "Nose", "C", "W", "J", "Medal", "Plane", "Five", "Hangman",
    "Spanish", "Siblings", "Snake", "Eight", "Mushroom", "Z", "Tetris", "Ear", "Bomb", "Bird", "L", "O", "Star"
};

static constexpr const char* sMoonNames[] {
    "Practice Mod", "Hi", "Practice", "Hi BTT", "BTT", "I'm cool", "Super Mario Odyssey", " "
};

static constexpr const char* sLanguageNames[] {
    "English", "日本語", "Deutsch", "Français"
};

static constexpr const char* sStickNames[] { "left", "right" };

static constexpr const char* sBodyNames[] {
    "Mario",          "Mario64",         "Mario64Metal",      "MarioAloha",
    "MarioArmor",     "MarioBone",       "MarioClown",        "MarioColorClassic",
    "MarioColorGold", "MarioColorLuigi", "MarioColorWaluigi", "MarioColorWario",
    "MarioCook",      "MarioDiddyKong",  "MarioDoctor",       "MarioExplorer",
    "MarioFootball",  "MarioGolf",       "MarioGunman",       "MarioHakama",
    "MarioHappi",     "MarioKing",       "MarioKoopa",        "MarioMaker",
    "MarioMechanic",  "MarioNew3DS",     "MarioPainter",      "MarioPeach",
    "MarioPilot",     "MarioPirate",     "MarioPoncho",       "MarioPrimitiveMan",
    "MarioSailor",    "MarioScientist",  "MarioShopman",      "MarioSnowSuit",
    "MarioSpaceSuit", "MarioSuit",       "MarioSwimwear",     "MarioTailCoat",
    "MarioTuxedo",    "MarioUnderwear"
};

static constexpr const char* sCapNames[] {
    "Mario",          "Mario64",         "Mario64Metal",      "MarioAloha",
    "MarioArmor",     "MarioBone",       "MarioClown",        "MarioColorClassic",
    "MarioColorGold", "MarioColorLuigi", "MarioColorWaluigi", "MarioColorWario",
    "MarioCook",      "MarioDiddyKong",  "MarioDoctor",       "MarioExplorer",
    "MarioFootball",  "MarioGolf",       "MarioGunman",       "MarioHakama",
    "MarioHappi",     "MarioKing",       "MarioKoopa",        "MarioMaker",
    "MarioMechanic",  "MarioNew3DS",     "MarioPainter",      "MarioPeach",
    "MarioPilot",     "MarioPirate",     "MarioPoncho",       "MarioPrimitiveMan",
    "MarioSailor",    "MarioScientist",  "MarioShopman",      "MarioSnowSuit",
    "MarioSpaceSuit", "MarioSuit",       "MarioSwimwear",     "MarioTailCoat",
    "MarioTuxedo",    "MarioInvisible"
};

static void applyOutfit(GameDataHolder* holder, const char* body, const char* cap)
{
    static void (*sWearCap)(GameDataHolder*, const char*) = nullptr;
    static void (*sWearCostume)(GameDataHolder*, const char*) = nullptr;
    if (!sWearCap) {
        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sWearCap), "_ZN16GameDataFunction7wearCapEP14GameDataHolderPKc");
    }
    if (!sWearCostume) {
        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sWearCostume), "_ZN16GameDataFunction11wearCostumeEP14GameDataHolderPKc");
    }
    if (sWearCap) sWearCap(holder, cap);
    if (sWearCostume) sWearCostume(holder, body);
}

static void applyDamagePlayer(GameDataHolder* holder)
{
    static void (*sDamagePlayer)(GameDataHolderWriter) = nullptr;
    if (!sDamagePlayer) {
        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sDamagePlayer), "_ZN16GameDataFunction12damagePlayerE20GameDataHolderWriter");
    }
    if (sDamagePlayer) sDamagePlayer(GameDataHolderWriter(holder));
}

Menu::Menu()
{
    // 0: Options (Refresh Settings + Toggles)
    mCategories[0].name = "options";
    mCategories[0].components.allocBuffer(23, nullptr);
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsMoonRefreshEnabled, "moonrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsGrayShineRefreshEnabled, "greymoonrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsPurpleCoinsRefreshEnabled, "purplecoinsrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsDoorRefreshEnabled, "doorsrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsMoonShardsRefreshEnabled, "shardsrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsKingdomEnterCutsceneRefreshEnabled, "kingdomenterrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsWarpTextRefreshEnabled, "warptextrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsSeedsRefreshEnabled, "seedsrefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsCutsceneRefreshEnabled, "cutscenerefresh"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsSeedsGrowing, "seedsgrowing"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsAlwaysManualCutscene, "alwaysmanualcutscene"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsAlwaysCheckpoints, "alwayscheckpoints"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsDisableMoonLock, "disablemoonlock"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsEnableAutosave, "disableautosave"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsDisableTpPuppet, "disabletppuppet"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsBgmDisabled, "disablemusic"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsNoDamageEnabled, "nodamage"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mDisableCheckpointTouching, "nocheckpointtouch"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsSkipCloudEnabled, "skipcloud"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsSkipBroodalsEnabled, "skipbroodals"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsAllCheckpointsEnabled, "allcheckpoints"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsLuigiPixelsEnabled, "luigipixels"));
    mCategories[0].components.pushBack(new BoolMenuComponent(&getConfig()->mIsHintPhotoSpawnEnabled, "spawnallhintart"));

    // 1: Stage Warp
    mCategories[1].name = "stage";
    mCategories[1].components.allocBuffer(3, nullptr);
    mCategories[1].components.pushBack(new EnumMenuComponent<int>(&getConfig()->mSelectedStageIdx, sStageNames, "Stage", false, false));
    mCategories[1].components.pushBack(new IntMenuComponent<int>(&getConfig()->mSelectedScenario, "scenario", 0, 15, true));
    mCategories[1].components.pushBack(new ButtonMenuComponent("go", [this]() {
        if (mScene && mScene->mIsAlive) {
            GameDataHolder* holder = (GameDataHolder*)al::getSceneObj(mScene, 20);
            if (holder) {
                int scenario = getConfig()->mSelectedScenario;
                if (scenario == 0) scenario = -1;
                ChangeStageInfo info = ChangeStageInfo(holder, "start", sStageNames[getConfig()->mSelectedStageIdx], false, scenario, { 0 });
                holder->changeNextStage(&info, 0);
            }
        }
    }, true));

    // 2: Misc
    mCategories[2].name = "misc";
    mCategories[2].components.allocBuffer(17, nullptr);
    mCategories[2].components.pushBack(new ButtonMenuComponent("saveposition", [this]() {
        if (mScene && mScene->mIsAlive) savePosition(rs::getPlayerActor(mScene));
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("loadposition", [this]() {
        if (mScene && mScene->mIsAlive) loadPosition(rs::getPlayerActor(mScene));
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("killmario", [this]() {
        callAction(ActionType::KillMario);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("damagemario", [this]() {
        if (mScene && mScene->mIsAlive) {
            GameDataHolder* holder = (GameDataHolder*)al::getSceneObj(mScene, 20);
            if (holder) {
                bool oldNoDmg = getConfig()->mIsNoDamageEnabled;
                getConfig()->mIsNoDamageEnabled = false;
                applyDamagePlayer(holder);
                getConfig()->mIsNoDamageEnabled = oldNoDmg;
            }
        }
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("lifeup", [this]() {
        callAction(ActionType::LifeMaxUp);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("healmario", [this]() {
        callAction(ActionType::HealMario);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("add1000", [this]() {
        callAction(ActionType::Add1000Coin);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("sub1000", [this]() {
        callAction(ActionType::Sub1000Coin);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("killscene", [this]() {
        callAction(ActionType::KillScene);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("prevscene", [this]() {
        callAction(ActionType::PrevScene);
    }, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("warplastcheckpoint", [this]() {
        callAction(ActionType::WarpCheckpoint);
    }, true));
    mCategories[2].components.pushBack(new BoolMenuComponent(&getConfig()->mIsNoclipEnabled, "noclip"));
    mCategories[2].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mCurPattern), sPatternNames, "wigglerpattern", false, true));
    mCategories[2].components.pushBack(new EnumMenuComponent<int>(&getConfig()->mMoonNameIdx, sMoonNames, "moonname", false, true));
    mCategories[2].components.pushBack(new EnumMenuComponent<int>(&getConfig()->mSelectedBody, sBodyNames, "outfitbody", false, true));
    mCategories[2].components.pushBack(new EnumMenuComponent<int>(&getConfig()->mSelectedCap, sCapNames, "outfitcap", false, true));
    mCategories[2].components.pushBack(new ButtonMenuComponent("setoutfit", [this]() {
        if (mScene && mScene->mIsAlive) {
            GameDataHolder* holder = (GameDataHolder*)al::getSceneObj(mScene, 20);
            if (holder) {
                applyOutfit(holder, sBodyNames[getConfig()->mSelectedBody], sCapNames[getConfig()->mSelectedCap]);
                mScene->kill();
            }
        }
    }, true));

    // 3: Timer (top-right by default, enabled by default, auto kingdom IL)
    mCategories[3].name = "timer";
    mCategories[3].components.allocBuffer(8, nullptr);
    mCategories[3].components.pushBack(new BoolMenuComponent(&getConfig()->mTimerEnabled, "timer"));
    mCategories[3].components.pushBack(new BoolMenuComponent(&getConfig()->mTimerAutoKingdom, "autokingdom"));
    mCategories[3].components.pushBack(new BoolMenuComponent(&getConfig()->mTimerSplit, "timersplit"));
    mCategories[3].components.pushBack(new ButtonMenuComponent("starttimer", []() {
        if (Timer::sInstance) Timer::sInstance->start();
    }, true));
    mCategories[3].components.pushBack(new ButtonMenuComponent("stoptimer", []() {
        if (Timer::sInstance) Timer::sInstance->stop();
    }, true));
    mCategories[3].components.pushBack(new ButtonMenuComponent("resettimer", []() {
        if (Timer::sInstance) Timer::sInstance->reset();
    }, true));
    mCategories[3].components.pushBack(new IntMenuComponent<float>(&getConfig()->mTimerFontSize, "fontsize", 8, 100, true));
    mCategories[3].components.pushBack(new Vector2MenuComponent(&getConfig()->mTimerPos, "position", true, ImVec2(0, 0), ImVec2(1600, 900)));

    // 4: Input Display
    mCategories[4].name = "inputdisplay";
    mCategories[4].components.allocBuffer(8, nullptr);
    mCategories[4].components.pushBack(new BoolMenuComponent(&getConfig()->mInputDisplayEnabled, "inputdisplay"));
    mCategories[4].components.pushBack(new BoolMenuComponent(&getConfig()->mInputDisplay2P, "2p_mode"));
    mCategories[4].components.pushBack(new Vector2MenuComponent(&getConfig()->mInputDisplayPos, "position", true, ImVec2(0, 0), ImVec2(1600, 900)));
    mCategories[4].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mInputDisplayButtonColor), sInputDisplayColorNames, "Button Color", false, false));
    mCategories[4].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mInputDisplayButtonPressedColor), sInputDisplayColorNames, "Pressed Color", false, false));
    mCategories[4].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mInputDisplayStickColor), sInputDisplayColorNames, "Stick Color", false, false));
    mCategories[4].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mInputDisplayRingColor), sInputDisplayColorNames, "Ring Color", false, false));
    mCategories[4].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mInputDisplayBackColor), sInputDisplayColorNames, "BG Color", false, false));

    // 5: Keybinds & Wheel
    mCategories[5].name = "keybinds";
    mCategories[5].components.allocBuffer(15, nullptr);
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mDUpBind), sActionNames, "dpadup", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mDDownBind), sActionNames, "dpaddown", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mDLeftBind), sActionNames, "dpadleft", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mDRightBind), sActionNames, "dpadright", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mRsLBind), sActionNames, "rsl", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mRsRBind), sActionNames, "rsr", true));
    mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mZLZRBind), sActionNames, "zlzr", true));

    static constexpr const char* wheelNames[] {
        "wheel1", "wheel2", "wheel3", "wheel4", "wheel5", "wheel6", "wheel7", "wheel8"
    };
    for (int i = 0; i < 8; i++) {
        mCategories[5].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->mQuickMenuBinds[i]), sActionNames, wheelNames[i], true));
    }

    // 6: Info
    mCategories[6].name = "info";
    mCategories[6].components.allocBuffer(4, nullptr);
    mCategories[6].components.pushBack(new BoolMenuComponent(&getConfig()->mIsInfoWindowEnabled, "infowindow"));
    mCategories[6].components.pushBack(new BoolMenuComponent(&getConfig()->mIsPlayerInfoEnabled, "playerinfo"));
    mCategories[6].components.pushBack(new Vector2MenuComponent(&getConfig()->mInfoPos, "position", true, ImVec2(0, 0), ImVec2(1600, 900)));
    mCategories[6].components.pushBack(new ButtonMenuComponent("resetinfopos", [this]() {
        getConfig()->mInfoPos = ImVec2(50.f, 50.f);
    }, true));

    // 7: Settings
    mCategories[7].name = "settings";
    mCategories[7].components.allocBuffer(3, nullptr);
    mCategories[7].components.pushBack(new EnumMenuComponent<int>(reinterpret_cast<int*>(&getConfig()->currentLanguage), sLanguageNames, "Language", false, false));
    mCategories[7].components.pushBack(new IntMenuComponent<int>(&getConfig()->mWheelDelayFrames, "wheeltime", 1, 40, true));
    mCategories[7].components.pushBack(new EnumMenuComponent<u8>(reinterpret_cast<u8*>(&getConfig()->mWheelActivatedPressRightStick), sStickNames, "wheelstick", true, true));

    mComponents.allocBuffer(4, nullptr);
    mComponents.pushBack(new QuickActionMenu(*this));
    mComponents.pushBack(new Timer);
    mComponents.pushBack(new MofumofuPatternUpdateNotification);
    mComponents.pushBack(new InputDisplay);
}

void Menu::update(al::Scene* scene)
{
    mScene = scene;

    if (mTimer % 3600 == 0) {
        pe::saveConfig();
    }
    mTimer++;

    if (scene && scene->mIsAlive && rs::getPlayerActor(scene) != nullptr) {
        StageScene* stageScene = static_cast<StageScene*>(scene);
        if (stageScene->mHolder) {
            static const char* (*sGetCurrentStageName)(GameDataHolderAccessor) = nullptr;
            static const char* (*sTryGetCurrentStageNameHolder)(GameDataHolder*) = nullptr;
            if (!sGetCurrentStageName && !sTryGetCurrentStageNameHolder) {
                nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetCurrentStageName), "_ZN16GameDataFunction19getCurrentStageNameE22GameDataHolderAccessor");
                nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sTryGetCurrentStageNameHolder), "_ZNK14GameDataHolder21tryGetCurrentStageNameEv");
            }
            const char* curStage = nullptr;
            if (sGetCurrentStageName) {
                curStage = sGetCurrentStageName(GameDataHolderAccessor(stageScene->mHolder));
            } else if (sTryGetCurrentStageNameHolder) {
                curStage = sTryGetCurrentStageNameHolder(stageScene->mHolder);
            }
            if (curStage && curStage[0] != '\0') {
                if (strncmp(mLastStageName, curStage, sizeof(mLastStageName)) != 0) {
                    strncpy(mLastStageName, curStage, sizeof(mLastStageName) - 1);
                    mLastStageName[sizeof(mLastStageName) - 1] = '\0';
                    auto* cfg = getConfig();
                    if (cfg && cfg->mTimerAutoKingdom && Timer::sInstance) {
                        Timer::sInstance->start();
                    }
                }
            }
        }
    }

    for (int i = 0; i < mComponents.size(); i++) {
        IComponent* component = mComponents[i];
        if (component)
            component->update();
    }
}

void Menu::draw()
{
    updateInput();

    ImGui::GetIO().FontGlobalScale = .5;

    for (int i = 0; i < mComponents.size(); i++) {
        IComponent* component = mComponents[i];
        if (component)
            component->draw();
    }

    drawInfoWindows();

    if (!mIsEnabled) {
        if (al::isPadTriggerUp(-1))
            callAction(getConfig()->mDUpBind);
        if (al::isPadTriggerDown(-1))
            callAction(getConfig()->mDDownBind);
        if (al::isPadTriggerLeft(-1) && !al::isPadHoldL(-1))
            callAction(getConfig()->mDLeftBind);
        if (al::isPadTriggerRight(-1))
            callAction(getConfig()->mDRightBind);
        if ((al::isPadHoldZL(-1) and al::isPadTriggerZR(-1)) or (al::isPadHoldZR(-1) and al::isPadTriggerZL(-1)))
            callAction(getConfig()->mZLZRBind);
        if (al::isPadHoldPressRightStick(-1) && al::isPadTriggerL(-1))
            callAction(getConfig()->mRsLBind);
        if (al::isPadHoldPressRightStick(-1) && al::isPadTriggerR(-1))
            callAction(getConfig()->mRsRBind);
        return;
    }

    mBgSize = ImVec2(0, 0);
    for (int i = 0; i < IM_ARRAYSIZE(mCategories); i++) {
        ImVec2 textSize = ImGui::GetIO().Fonts->Fonts[0]->CalcTextSizeA(sCategoryFontSize, FLT_MAX, 0.f, pe::getLocalizedString(mCategories[i].name));

        int width = sCategorySpacing + textSize.x + sCategorySpacing;
        mBgSize.x = std::max(mBgSize.x, (float)width);
    }
    mBgSize.y += getCategoryHeight(IM_ARRAYSIZE(mCategories));

    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), mBgSize, IM_COL32(0, 0, 0, 128));
    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, getCategoryHeight(mCurrentCategory)), ImVec2(mBgSize.x, getCategoryHeight(mCurrentCategory + 1)), IM_COL32(255, 255, 255, 76));

    ImVec2 size = ImVec2(0, 0);
    for (int i = 0; i < IM_ARRAYSIZE(mCategories); i++) {
        ImVec2 textSize = ImGui::GetIO().Fonts->Fonts[0]->CalcTextSizeA(sCategoryFontSize, FLT_MAX, 0.f, pe::getLocalizedString(mCategories[i].name));
        size.y += sCategorySpacing;

        ImGui::GetForegroundDrawList()->AddText(ImGui::GetIO().Fonts->Fonts[0], sCategoryFontSize, ImVec2(size.x + sCategorySpacing, size.y), IM_COL32(255, 255, 255, 255), pe::getLocalizedString(mCategories[i].name));

        size.y += textSize.y;
        size.y += sCategorySpacing;
    }

    if (mIsExpandedCurrentCategory)
        drawExpandedCategory();
}

void Menu::drawExpandedCategory()
{
    auto& cat = mCategories[mCurrentCategory];
    if (mCurrentComponentInCategory >= cat.components.size())
        mCurrentComponentInCategory = cat.components.size() - 1;
    int subX = 0, subY = 0;
    int y = getCategoryHeight(mCurrentCategory);

    for (int i = 0; i < cat.components.size(); i++) {
        MenuComponent* component = cat.components[i];
        if (component == nullptr)
            continue;
        ImVec2 size = component->getSize();
        subX = std::max((float)subX, size.x);
        subY += size.y;
    }

    getCurrentCategory().components[mCurrentComponentInCategory]->setIsHovered(true);

    subY = 0;
    {
        for (int i = 0; i < cat.components.size(); i++) {
            MenuComponent* component = cat.components[i];
            if (component == nullptr)
                continue;

            ImVec2 size = component->getSize();
            ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(mBgSize.x, y + subY), ImVec2(mBgSize.x + subX, y + subY + size.y), IM_COL32(0, 0, 0, 128), 15);
            if (mCurrentComponentInCategory == i) {
                ImU32 color = mIsFocusedOnCurrentComponent ? IM_COL32(255, 60, 60, 76) : IM_COL32(255, 255, 255, 76);
                ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(mBgSize.x, y + subY), ImVec2(mBgSize.x + subX, y + subY + size.y), color, 15);
            }
            component->draw(ImVec2(mBgSize.x, y + subY));
            component->setIsHovered(false);

            subY += size.y;
        }
    }
}

void Menu::drawInfoWindows()
{
    auto* cfg = getConfig();
    if (!cfg) return;

    if (cfg->mIsInfoWindowEnabled) {
        ImGui::SetNextWindowPos(cfg->mInfoPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Info Window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
            if (mScene && mScene->mIsAlive) {
                GameDataHolder* holder = reinterpret_cast<GameDataHolder*>(al::getSceneObj(mScene, 20));
                if (holder) {
                    static s32 (*sGetJumpCount)(void*) = nullptr;
                    static s32 (*sGetThrowCapCount)(void*) = nullptr;
                    static s32 (*sGetTotalCoinNum)(void*) = nullptr;
                    static u64 (*sGetPlayTimeTotal)(GameDataHolderAccessor) = nullptr;
                    static u64 (*sGetPlayTimeAcrossFile)(GameDataHolderAccessor) = nullptr;

                    if (!sGetJumpCount) {
                        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetJumpCount), "_ZN2rs18getPlayerJumpCountEP14GameDataHolder");
                        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetThrowCapCount), "_ZN2rs22getPlayerThrowCapCountEP14GameDataHolder");
                        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetTotalCoinNum), "_ZN2rs15getTotalCoinNumEP14GameDataHolder");
                        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetPlayTimeTotal), "_ZN16GameDataFunction16getPlayTimeTotalE22GameDataHolderAccessor");
                        nn::ro::LookupSymbol(reinterpret_cast<uintptr_t*>(&sGetPlayTimeAcrossFile), "_ZN16GameDataFunction21getPlayTimeAcrossFileE22GameDataHolderAccessor");
                    }

                    s32 jumps = sGetJumpCount ? sGetJumpCount(holder) : 0;
                    s32 throws = sGetThrowCapCount ? sGetThrowCapCount(holder) : 0;
                    s32 coins = sGetTotalCoinNum ? sGetTotalCoinNum(holder) : 0;
                    u64 totalTime = sGetPlayTimeTotal ? sGetPlayTimeTotal(GameDataHolderAccessor(holder)) : 0;
                    u64 acrossTime = sGetPlayTimeAcrossFile ? sGetPlayTimeAcrossFile(GameDataHolderAccessor(holder)) : 0;

                    ImGui::Text("Jumps: %d", jumps);
                    ImGui::Text("Cap Throws: %d", throws);
                    ImGui::Text("Total Coins: %d", coins);
                    ImGui::Text("Play Time Total: %lu", totalTime);
                    ImGui::Text("Play Time Across File: %lu", acrossTime);
                } else {
                    ImGui::Text("Waiting for stage...");
                }
            }
            ImGui::End();
        }
    }

    if (cfg->mIsPlayerInfoEnabled && mScene && mScene->mIsAlive) {
        PlayerActorBase* playerBase = reinterpret_cast<PlayerActorBase*>(rs::getPlayerActor(mScene));
        if (playerBase) {
            ImGui::SetNextWindowPos(ImVec2(cfg->mInfoPos.x, cfg->mInfoPos.y + 220.f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(340, 220), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Player Info", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
                al::LiveActor* actor = reinterpret_cast<al::LiveActor*>(playerBase);
                sead::Vector3f pos = al::getTrans(actor);
                static sead::Vector3f sLastPos(0, 0, 0);
                sead::Vector3f vel = (pos - sLastPos) * 60.0f;
                sLastPos = pos;
                float hSpeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
                float vSpeed = vel.y;
                float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
                ImGui::Text("Pos:   %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
                ImGui::Text("Vel:   %.1f, %.1f, %.1f", vel.x, vel.y, vel.z);
                ImGui::Text("Speed: %.2f (H: %.2f, V: %.2f)", speed, hSpeed, vSpeed);
                ImGui::End();
            }
        }
    }
}

void Menu::updateInput()
{
    if (al::isPadHoldL(-1) && al::isPadTriggerLeft(-1)) {
        mIsEnabled = !mIsEnabled;
        return;
    }

    if (!mIsExpandedCurrentCategory) {
        if (al::isPadTriggerDown(-1)) {
            mCurrentCategory++;
        } else if (al::isPadTriggerUp(-1)) {
            mCurrentCategory--;
        }

        if (al::isPadTriggerRight(-1)) {
            mIsExpandedCurrentCategory = true;
        }
    } else {
        if (!mIsFocusedOnCurrentComponent) {
            if (al::isPadTriggerDown(-1)) {
                mCurrentComponentInCategory++;
            } else if (al::isPadTriggerUp(-1)) {
                mCurrentComponentInCategory--;
            } else if (al::isPadTriggerLeft(-1)) {
                mIsExpandedCurrentCategory = false;
            }
        }

        if (mCurrentComponentInCategory < 0)
            mCurrentComponentInCategory = 0;
        if (mCurrentComponentInCategory >= getCurrentCategory().components.size())
            mCurrentComponentInCategory = getCurrentCategory().components.size() - 1;

        if (getCurrentCategory().components[mCurrentComponentInCategory]->canHaveFocus() && mIsEnabled) {
            if (al::isPadTriggerA(-1))
                mIsFocusedOnCurrentComponent = !mIsFocusedOnCurrentComponent;
            getCurrentCategory().components[mCurrentComponentInCategory]->setIsFocused(mIsFocusedOnCurrentComponent);
        } else
            mIsFocusedOnCurrentComponent = false;
    }

    if (mCurrentCategory >= IM_ARRAYSIZE(mCategories))
        mCurrentCategory = IM_ARRAYSIZE(mCategories) - 1;
    if (mCurrentCategory < 0)
        mCurrentCategory = 0;
}

void Menu::savePosition(al::LiveActor* playerBase)
{
    if (PlayerActorHakoniwa* player = util::typeCast<PlayerActorHakoniwa>((al::LiveActor*)playerBase)) {
        al::LiveActor* hack = player->getCurrentHack();
        al::LiveActor* move = hack != nullptr ? hack : player;

        mSavedPosition = al::getTrans(move);
        mSavedQuat = move->getPoseKeeper()->getQuat();
        mIsSavedPos = true;
    }
}

void Menu::loadPosition(al::LiveActor* playerBase)
{
    if (!mIsSavedPos)
        return;
    if (PlayerActorHakoniwa* player = util::typeCast<PlayerActorHakoniwa>((al::LiveActor*)playerBase)) {
        al::LiveActor* hack = player->getCurrentHack();
        al::LiveActor* move = hack != nullptr ? hack : player;

        if (hack == nullptr)
            player->startDemoPuppetableSuperReal();

        al::offCollide(move);

        al::setTrans(move, mSavedPosition);
        al::updatePoseQuat(move, mSavedQuat);

        if (hack == nullptr)
            player->endDemoPuppetableSuperReal();

        al::onCollide(move);
    }
}

int Menu::getCategoryHeight(int id)
{
    int height = 0;
    for (int i = 0; i < id; i++) {
        ImVec2 textSize = ImGui::GetIO().Fonts->Fonts[0]->CalcTextSizeA(sCategoryFontSize, FLT_MAX, 0.f, pe::getLocalizedString(mCategories[i].name));
        height += sCategorySpacing + textSize.y + sCategorySpacing;
    }
    return height;
}

void Menu::callAction(ActionType type)
{
    if (mScene && mScene->mIsAlive) {
        const al::Nerve* nrv = mScene->getNerveKeeper()->getCurrentNerve();
        bool allowedNerve = (nrv == util::getNerveAt(offsets::StageSceneNrvPlay) || nrv == util::getNerveAt(offsets::StageSceneNrvShineGet));
        uintptr_t typeInfo = *reinterpret_cast<uintptr_t*>(pe::util::getVft(nrv) - 8);
        const char** typeName = reinterpret_cast<const char**>(typeInfo + 8);
        if (strstr(*typeName, "StageSceneNrvDemo") or allowedNerve) {
            PlayerActorBase* playerBase = reinterpret_cast<PlayerActorBase*>(rs::getPlayerActor(mScene));
            switch (type) {
            case ActionType::KillScene: {
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));

                mScene->kill();
                return;
            }
            case ActionType::PrevScene: {
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));

                StageScene* scene = static_cast<StageScene*>(mScene);
                scene->mHolder->returnPrevStage();
                return;
            }
            case ActionType::SavePosition: {
                if (playerBase)
                    savePosition(playerBase);

                return;
            }
            case ActionType::LoadPosition: {
                if (playerBase)
                    loadPosition(playerBase);
                return;
            }
            case ActionType::LifeMaxUp: {
                GameDataFunction::getLifeMaxUpItem(static_cast<PlayerActorHakoniwa*>(playerBase));
                return;
            }
            case ActionType::HealMario:
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));
                return;
            case ActionType::Add1000Coin: {
                StageScene* scene = static_cast<StageScene*>(mScene);
                scene->mHolder->mPlayingFile->addCoin(1000);
                return;
            }
            case ActionType::Sub1000Coin: {
                StageScene* scene = static_cast<StageScene*>(mScene);
                scene->mHolder->mPlayingFile->addCoin(-1000);
                return;
            }
            case ActionType::WarpCheckpoint: {
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));
                GameDataFunction::recoveryPlayer(static_cast<PlayerActorHakoniwa*>(playerBase));

                StageScene* scene = static_cast<StageScene*>(mScene);
                if (mLastMapTarget) {
                    scene->mStateCollection->mStateStageMap->mMapThing = mLastMapTarget;
                    al::setNerve(scene, pe::util::getNerveAt(offsets::StageSceneNrvWarpToCheckpoint));
                }
                return;
            }
            case ActionType::KillMario: {
                if (PlayerActorHakoniwa* player = util::typeCast<PlayerActorHakoniwa>((al::LiveActor*)playerBase)) {
                    player->mPlayerDamageKeeper->dead();
                }
                return;
            }
            case ActionType::ToggleNoclip: {
                getConfig()->mIsNoclipEnabled = !getConfig()->mIsNoclipEnabled;
                return;
            }
            default:
                break;
            }
        }
    }

    switch (type) {
    case ActionType::StartTimer:
        if (Timer::sInstance) Timer::sInstance->start();
        return;
    case ActionType::StopTimer:
        if (Timer::sInstance) Timer::sInstance->stop();
        return;
    case ActionType::ResetTimer:
        if (Timer::sInstance) Timer::sInstance->reset();
        return;
    case ActionType::NextMofumofuPattern: {
        int pattern = (int)getConfig()->mCurPattern + 1;
        if (pattern > 22) pattern = 0;
        getConfig()->mCurPattern = (MofumofuPattern)pattern;
        return;
    }
    case ActionType::PrevMofumofuPattern: {
        int pattern = (int)getConfig()->mCurPattern - 1;
        if (pattern < 0) pattern = 22;
        getConfig()->mCurPattern = (MofumofuPattern)pattern;
        return;
    }
    default:
        break;
    }
}

} // namespace pe

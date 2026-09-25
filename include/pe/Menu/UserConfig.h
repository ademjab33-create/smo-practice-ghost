#pragma once

#include "imgui.h"
#include "pe/Menu/Action.h"
#include "pe/Menu/InputDisplay.h"
#include "pe/Menu/Timer.h"
#include "pe/Util/Localization.h"

namespace pe {

enum MofumofuPattern : s8 {
    Random,
    Ghost,
    Nose,
    C,
    W,
    J,
    Medal,
    Plane,
    Five,
    Hangman,
    Spanish,
    Siblings,
    Snake,
    Eight,
    Mushroom,
    Z,
    Tetris,
    Ear,
    Bomb,
    Bird,
    L,
    O,
    Star
};

struct UserConfig {
    PeepaLanguage currentLanguage = PeepaLanguage::English;
    ActionType mDUpBind = ActionType::KillScene;
    ActionType mDDownBind = ActionType::PrevScene;
    ActionType mDLeftBind = ActionType::SavePosition;
    ActionType mDRightBind = ActionType::LoadPosition;
    ActionType mQuickMenuBinds[8] {
        ActionType::SavePosition, ActionType::LoadPosition, ActionType::HealMario, ActionType::Add1000Coin,
        ActionType::KillScene, ActionType::PrevScene, ActionType::StartTimer, ActionType::ResetTimer
    };
    ActionType mRsLBind = ActionType::None;
    ActionType mRsRBind = ActionType::None;
    ActionType mZLZRBind = ActionType::KillScene;

    // Timer config: top-right by default, enabled by default, auto kingdom IL
    ImVec2 mTimerPos = ImVec2(1250.f, 35.f);
    bool mTimerEnabled = true;
    bool mTimerAutoKingdom = true;
    bool mTimerIsRTA = true;
    bool mTimerSplit = true;
    float mTimerFontSize = 50.0f;
    TimerHookType mTimerStartType = TimerHookType::None;

    // Input display
    bool mInputDisplayEnabled = true;
    bool mInputDisplay2P = false;
    ImVec2 mInputDisplayPos = ImVec2(1600.f / 2, 900.f / 2);
    InputDisplayColor mInputDisplayButtonColor = InputDisplayColor::White;
    InputDisplayColor mInputDisplayButtonPressedColor = InputDisplayColor::Pink;
    InputDisplayColor mInputDisplayStickColor = InputDisplayColor::White;
    InputDisplayColor mInputDisplayRingColor = InputDisplayColor::Gray;
    InputDisplayColor mInputDisplayBackColor = InputDisplayColor::Black;

    // Refresh Settings (from Practice 1.0)
    bool mIsMoonRefreshEnabled = false;
    bool mIsGrayShineRefreshEnabled = false;
    bool mIsPurpleCoinsRefreshEnabled = false;
    bool mIsDoorRefreshEnabled = false;
    bool mIsMoonShardsRefreshEnabled = false;
    bool mIsKingdomEnterCutsceneRefreshEnabled = false;
    bool mIsWarpTextRefreshEnabled = false;
    bool mIsSeedsRefreshEnabled = false;
    bool mIsCutsceneRefreshEnabled = false;

    // Gameplay & Disable Toggles (from Practice 1.0)
    bool mIsSeedsGrowing = false;
    bool mIsAlwaysManualCutscene = false;
    bool mIsAlwaysCheckpoints = false;
    bool mIsDisableMoonLock = false;
    bool mIsEnableAutosave = true;
    bool mIsDisableTpPuppet = false;
    bool mIsBgmDisabled = false;
    bool mIsNoDamageEnabled = false;
    bool mDisableCheckpointTouching = false;
    bool mIsSkipCloudEnabled = false;
    bool mIsSkipBroodalsEnabled = false;
    bool mIsAllCheckpointsEnabled = false;
    bool mIsLuigiPixelsEnabled = false;
    bool mIsHintPhotoSpawnEnabled = false;

    // Misc (from Practice 1.0 & 1.3)
    MofumofuPattern mCurPattern = Random;
    bool mIsNoclipEnabled = false;
    int mMoonNameIdx = 0;
    int mSelectedBody = 0;
    int mSelectedCap = 0;
    bool mIsInfoWindowEnabled = false;
    bool mIsPlayerInfoEnabled = false;
    ImVec2 mInfoPos = ImVec2(50.f, 50.f);

    // Stage
    int mSelectedStageIdx = 0;
    int mSelectedScenario = 0;

    // Wheel
    int mWheelDelayFrames = 1;
    bool mWheelActivatedPressRightStick = false;

    struct MofumofuPatternEntry {
        const char* typeStr;
        int target;
        bool reverse;
    };

    constexpr static const MofumofuPatternEntry sPatternEntries[22] = {
        { "Ghost", 0, false }, { "Nose", 0, true }, { "C", 1, false }, { "W", 1, true }, { "J", 2, false }, { "Medal", 2, true }, { "Plane", 3, false }, { "5", 3, true }, { "Hangman", 4, false }, { "Spanish", 4, true },
        { "Siblings", 5, false }, { "Snake", 5, true }, { "8", 6, false }, { "Mushroom", 6, true }, { "Z", 7, false }, { "Tetris", 7, true }, { "Ear", 8, false }, { "Bomb", 8, true }, { "Bird", 9, false }, { "L", 9, true }, { "O", 10, false }, { "Star", 10, true }
    };
};

UserConfig*& getConfig();
void saveConfig();
void loadConfig();

} // namespace pe

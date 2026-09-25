#include "pe/Menu/Timer.h"
#include "al/Library/LiveActor/ActorClippingFunction.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "nn/os.h"
#include "patch/code_patcher.hpp"
#include "pe/Menu/UserConfig.h"
#include "pe/Util/Offsets.h"
#include "pe/Ghost/ILTracker.h"
#include <cstdio>

namespace pe {

Timer* Timer::sInstance = nullptr;

static void shineGrabHook(al::LiveActor* shine)
{
    al::invalidateClipping(shine);
    Timer::sInstance->event(TimerHookType::ShineGrab);
    if (ILTracker::instance()) {
        ILTracker::instance()->notifyMoonStory();
    }
}

static bool shineTickHook(al::LiveActor* actor)
{
    bool isFirstStep = al::isFirstStep(actor);
    if (actor->getNerveKeeper()->getCurrentStep() == 1) {
        Timer::sInstance->event(TimerHookType::ShineTick);
    }
    return isFirstStep;
}

Timer::Timer()
{
    sInstance = this;

#if GAME_VERSION == 130
    using Patcher = exl::patch::CodePatcher;
    Patcher(offsets::ShineGrabHook).BranchLinkInst((void*)shineGrabHook);
    Patcher(offsets::ShineTickHook).BranchLinkInst((void*)shineTickHook);
#endif
}

void Timer::start()
{
    mStartTick = nn::os::GetSystemTick();
    mIsRunning = true;
    if (ILTracker::instance()) {
        ILTracker::instance()->startRun();
    }
}

void Timer::stop()
{
    mIsRunning = false;
    if (ILTracker::instance()) {
        ILTracker::instance()->stopRun();
    }
}

void Timer::reset()
{
    mIsRunning = false;
    mStartTick = 0;
    mElapsedTicks = 0;
    if (ILTracker::instance()) {
        ILTracker::instance()->resetRun();
    }
}

void Timer::event(TimerHookType type)
{
    if (getConfig()->mTimerStartType == (int)type) {
        start();
    }
    if (getConfig()->mTimerSplit && mIsRunning) {
        stop();
    }
}

void Timer::update()
{
    if (mIsRunning) {
        mElapsedTicks = nn::os::GetSystemTick() - mStartTick;
    }
}

void Timer::draw()
{
    if (!getConfig()->mTimerEnabled)
        return;

    char buf[64];
    s64 ms = (mElapsedTicks * 1000) / 19200000;
    s32 totalSec = ms / 1000;
    s32 min = totalSec / 60;
    s32 sec = totalSec % 60;
    s32 remMs = ms % 1000;

    snprintf(buf, sizeof(buf), "%02d:%02d.%03d", min, sec, remMs);

    ImVec2 pos = getConfig()->mTimerPos;
    ImGui::GetForegroundDrawList()->AddText(ImGui::GetIO().Fonts->Fonts[0], getConfig()->mTimerFontSize, pos, IM_COL32(255, 255, 255, 255), buf);
}

} // namespace pe
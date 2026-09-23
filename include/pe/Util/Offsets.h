#pragma once
#include <cstdint>

namespace pe {
namespace offsets {

#if GAME_VERSION == 100
    const uintptr_t FileDeviceMgrCtorHookLocation = 0x0076c8d4;
    const uintptr_t HakoniwaSequenceInitHookLocation = 0x0050e6a8;
    const uintptr_t HakoniwaSequenceUpdate = 0x0050f030;
    const uintptr_t SceneEndInitHookLocation = 0x009cea7c;
#else
    const uintptr_t FileDeviceMgrCtorHookLocation = 0x00943d60;
    const uintptr_t HakoniwaSequenceInitHookLocation = 0x004b4230;
    const uintptr_t HakoniwaSequenceUpdate = 0x004b4e90;
    const uintptr_t SceneEndInitHookLocation = 0x004b4e90;
#endif

} // namespace offsets
} // namespace pe
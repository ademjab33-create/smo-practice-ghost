#pragma once

#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Scene/Scene.h"
#include "container/seadPtrArray.h"
#include "heap/seadHeap.h"
#include "imgui.h"
#include "pe/Menu/Action.h"
#include "pe/Menu/IComponent.h"
#include "pe/Menu/MenuComponent.h"
#include "pe/Menu/UserConfig.h"
#include <sead/heap/seadDisposer.h>

namespace pe {

sead::Heap*& getMenuHeap();

class Menu : public sead::IDisposer, public IComponent {
    SEAD_SINGLETON_DISPOSER(Menu)

public:
    static constexpr int sCategorySpacing = 6;
    static constexpr int sCategoryFontSize = 40;

    Menu();
    void update(al::Scene* scene);
    void updateInput();
    void draw() override;
    void drawExpandedCategory();
    void drawInfoWindows();
    bool isEnabled() const { return mIsEnabled; }

    void savePosition(al::LiveActor* player);
    void loadPosition(al::LiveActor* player);
    void setLatestMapTarget(void* target) { mLastMapTarget = target; }
    void callAction(ActionType type);

    struct Category {
        const char* name;
        sead::PtrArray<MenuComponent> components;
    };

    Category& getCurrentCategory() { return mCategories[mCurrentCategory]; }
    int getCategoryHeight(int id);

private:
    bool mIsEnabled = false;
    int mCurrentCategory = 0;
    int mCurrentComponentInCategory = 0;
    bool mIsExpandedCurrentCategory = false;
    bool mIsFocusedOnCurrentComponent = false;

    Category mCategories[9];
    sead::PtrArray<IComponent> mComponents;

    ImVec2 mBgSize = ImVec2(0, 0);
    sead::Vector3f mSavedPosition = sead::Vector3f(0, 0, 0);
    sead::Quatf mSavedQuat = sead::Quatf(0, 0, 0, 0);
    bool mIsSavedPos = false;
    void* mLastMapTarget = nullptr;

    al::Scene* mScene = nullptr;
    int mTimer = 0;
};

} // namespace pe

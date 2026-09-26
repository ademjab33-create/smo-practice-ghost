#include "pe/Menu/InputDisplay.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include "imgui.h"
#include "pe/Menu/UserConfig.h"

namespace pe {

using InputCallback = bool (*)(int port);

static ImU32 makeColor(const ImVec4& color) { return IM_COL32(u8(color.x), u8(color.y), u8(color.z), u8(color.w)); }

static void drawButton(int port, const ImVec2& pos, InputCallback callback, float radius = 8, ImU32 color = makeColor(getInputDisplayColor(getConfig()->mInputDisplayButtonColor)), ImU32 pressedColor = makeColor(getInputDisplayColor(getConfig()->mInputDisplayButtonPressedColor)))
{
    const ImU32 col = callback(port) ? pressedColor : color;
    ImGui::GetForegroundDrawList()->AddCircleFilled(pos, radius, col, 16);
}

static void drawButtonRect(int port, const ImVec2& pos, InputCallback callback, ImU32 color = makeColor(getInputDisplayColor(getConfig()->mInputDisplayButtonColor)), ImU32 pressedColor = makeColor(getInputDisplayColor(getConfig()->mInputDisplayButtonPressedColor)))
{
    const ImU32 col = callback(port) ? pressedColor : color;
    const ImVec2 min(pos.x - 13, pos.y - 6);
    const ImVec2 max(pos.x + 13, pos.y + 6);
    ImGui::GetForegroundDrawList()->AddRectFilled(min, max, col, 10.0f);
}

static void drawSingleController(int port, ImVec2 basePos)
{
    const sead::Vector2f leftStick = al::getLeftStick(port);
    const sead::Vector2f rightStick = al::getRightStick(port);

    ImVec2 pos = basePos;
    pos.x -= 200;
    pos.y -= 100;

    if (getConfig()->mInputDisplayBackColor != InputDisplayColor::None) {
        ImVec4 color = getInputDisplayColor(getConfig()->mInputDisplayBackColor);
        color.w = 128;
        ImGui::GetForegroundDrawList()->AddRectFilled({ pos.x - 50, pos.y - 100 }, { pos.x + 200, pos.y + 100 }, makeColor(color), 20);
    }

    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(getConfig()->mInputDisplayRingColor)), 0, 2);
    ImVec2 leftPos = { pos.x + leftStick.x * 30, pos.y - leftStick.y * 30 };
    drawButton(port, leftPos, al::isPadHoldPressLeftStick, 16, makeColor(getInputDisplayColor(getConfig()->mInputDisplayStickColor)));

    pos.x += 40;
    pos.y += 30;
    drawButton(port, pos, al::isPadHoldUp);
    pos.y += 30;
    drawButton(port, pos, al::isPadHoldDown);
    pos.x -= 15;
    pos.y -= 15;
    drawButton(port, pos, al::isPadHoldLeft);
    pos.x += 30;
    drawButton(port, pos, al::isPadHoldRight);

    pos.x += 60;
    ImGui::GetForegroundDrawList()->AddCircle(pos, 25, makeColor(getInputDisplayColor(getConfig()->mInputDisplayRingColor)), 0, 2);
    ImVec2 rightPos = { pos.x + rightStick.x * 30, pos.y - rightStick.y * 30 };
    drawButton(port, rightPos, al::isPadHoldPressRightStick, 16, makeColor(getInputDisplayColor(getConfig()->mInputDisplayStickColor)));

    pos.x += 40;
    pos.y -= 60;
    drawButton(port, pos, al::isPadHoldX);
    pos.y += 30;
    drawButton(port, pos, al::isPadHoldB);
    pos.x -= 15;
    pos.y -= 15;
    drawButton(port, pos, al::isPadHoldY);
    pos.x += 30;
    drawButton(port, pos, al::isPadHoldA);

    pos.y -= 10;
    pos.x -= 75;
    drawButton(port, pos, al::isPadHoldPlus, 5);
    pos.x -= 40;
    drawButton(port, pos, al::isPadHoldMinus, 5);

    ImVec2 trigPos = basePos;
    trigPos.x -= 200;
    trigPos.y -= 160;
    drawButtonRect(port, trigPos, al::isPadHoldL);
    trigPos.y -= 16;
    drawButtonRect(port, trigPos, al::isPadHoldZL);
    trigPos.x += 155;
    drawButtonRect(port, trigPos, al::isPadHoldZR);
    trigPos.y += 16;
    drawButtonRect(port, trigPos, al::isPadHoldR);
}

void InputDisplay::draw()
{
    if (!getConfig()->mInputDisplayEnabled)
        return;

    drawSingleController(-1, getConfig()->mInputDisplayPos);

    if (getConfig()->mInputDisplay2P) {
        ImVec2 p2Pos = getConfig()->mInputDisplayPos;
        p2Pos.x += 280;
        drawSingleController(1, p2Pos);
    }
}

} // namespace pe

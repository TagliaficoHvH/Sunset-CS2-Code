#pragma once

#include "../external/ImGui/imgui.h"

typedef struct
{
    ImU32 R;
    ImU32 G;
    ImU32 B;
} RGB;

struct RGBs {
    float r, g, b;
};

ImU32 Color(RGB color)
{
    return IM_COL32(color.R, color.G, color.B, 255); // Using ImU32 color format for ImGui
}

ImU32 Color(RGBs color)
{
    return IM_COL32(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255), static_cast<int>(color.b * 255), 255); // Using ImU32 color format for ImGui
}

namespace Render
{
    // Updated DrawRect function
    void DrawRect(int x, int y, int w, int h, RGBs color, int thickness)
    {
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), Color(color), 0, 0, thickness);
    }

    // Updated Line function
    void Line(float x1, float y1, float x2, float y2, RGBs color, float thickness)
    {
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), Color(color), thickness);
    }

    // Updated Circle function
    void Circle(float x, float y, float radius, RGBs color)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), radius, Color(color), 0, 1);
    }

    // Updated DrawText function
    void DrawText(float x, float y, const char* text, RGBs color)
    {
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), Color(color), text);
    }
}
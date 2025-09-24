#include "Watermark.h"

#include <chrono>
#include <string>

bool watermarkEnabled = false;
float watermarkTransparency = 0.5f; // Default transparency value
ImVec4 watermarkTextColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange (255, 128, 0)
ImVec4 watermarkBackGroundColor = ImVec4(25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f); // Gray (64, 64, 64)

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    struct tm time_info;
    localtime_s(&time_info, &now_time);
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &time_info);
    return std::string(buffer);
}

void RenderWatermark(int ping, int fps) {
    if (watermarkEnabled) {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::SetNextWindowBgAlpha(watermarkTransparency);

        ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f);
        ImVec2 windowPosPivot = ImVec2(1.0f, 0.0f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(watermarkBackGroundColor.x, watermarkBackGroundColor.y, watermarkBackGroundColor.z, watermarkTransparency));
        ImGui::Begin("Watermark", nullptr, windowFlags);
        std::string watermarkText = "SunSet / " + getCurrentTime();

        watermarkText += " / FPS: " + std::to_string(fps) + " / Ping: " + std::to_string(ping) + "ms";

        ImGui::TextColored(watermarkTextColor, "%s", watermarkText.c_str());
        ImGui::End();
        ImGui::PopStyleColor();
    }
}
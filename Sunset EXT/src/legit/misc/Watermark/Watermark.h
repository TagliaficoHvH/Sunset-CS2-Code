#ifndef WATERMARK_H
#define WATERMARK_H

#include "../../../../external/ImGui/imgui.h"

extern bool watermarkEnabled;
extern float watermarkTransparency;
extern ImVec4 watermarkTextColor;
extern ImVec4 watermarkBackGroundColor;

void RenderWatermark(int ping, int fps);

#endif // WATERMARK_H

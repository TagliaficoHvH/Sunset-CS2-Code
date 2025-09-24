#pragma once

#include "../../../cs/memory.h"
#include "../../../../external/ImGui/imgui.h"

extern bool triggerbotEnabled;
extern int triggerbotDelay;
extern ImGuiKey triggerbotKey;
extern enum TriggerbotMode { Always, Hold, Toggle };
extern TriggerbotMode triggerbotMode;

void HandleTriggerBot(Memory& mem, std::uintptr_t client);

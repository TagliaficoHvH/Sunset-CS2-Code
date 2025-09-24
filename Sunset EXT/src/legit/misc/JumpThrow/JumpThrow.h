#pragma once

#include "../../../cs/memory.h"
#include "../../../../external/ImGui/imgui.h"


extern bool JumpThrow;
extern ImGuiKey JumpKey; // Key for JumpThrow

void HandleJumpThrow(Memory& mem, std::uintptr_t client);

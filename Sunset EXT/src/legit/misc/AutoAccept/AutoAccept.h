#pragma once

#include "../../../cs/memory.h"

extern bool AutoAccept;
extern bool g_inGame;

void AutoAcceptMatchByQueue(Memory& mem, std::uintptr_t client);
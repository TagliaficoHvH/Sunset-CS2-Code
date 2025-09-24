#pragma once

#include "../../../cs/memory.h"

extern bool NoFlash;

void HandleNoFlash(Memory& mem, std::uintptr_t client);

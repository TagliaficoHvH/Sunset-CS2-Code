#include <thread>
#include "NoFlash.h"
#include "../../../cs/offsets.h"

bool NoFlash = false;

void HandleNoFlash(Memory& mem, std::uintptr_t client) {
    while (true) {
        if (NoFlash) {
            uintptr_t localPlayer = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
            float flashDuration = mem.Read<float>(localPlayer + offset::m_flFlashBangTime);

            if (flashDuration > 0) {
                mem.Write(localPlayer + offset::m_flFlashBangTime, 0.0f);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
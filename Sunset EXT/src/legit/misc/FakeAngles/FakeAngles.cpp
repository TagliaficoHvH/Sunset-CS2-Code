#include "FakeAngles.h"
#include "../../../cs/vector.h"
#include "../../../cs/offsets.h"
#include <thread>

bool fakeAngles = false;

void HandleFakeAngles(Memory& mem, std::uintptr_t client) {
    while (true) {
        if (fakeAngles) {
            uintptr_t localPlayer = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
            Vector3 viewAngles = mem.Read<Vector3>(localPlayer + offset::dwViewAngles);

            Vector3 newAngles = {
                viewAngles.x,
                viewAngles.y - 89.0f,
                viewAngles.z
            };

            mem.Write(localPlayer + offset::v_angle, newAngles);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
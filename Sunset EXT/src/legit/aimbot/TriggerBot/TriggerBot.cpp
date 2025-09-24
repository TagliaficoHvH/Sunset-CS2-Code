#include <Windows.h>
#include <thread>
#include "triggerbot.h"
#include "../../../cs/offsets.h"
#include "../../../cs/weapon_index.h"

bool triggerbotEnabled = false;
int triggerbotDelay = 1;
TriggerbotMode triggerbotMode = Always;
ImGuiKey triggerbotKey = ImGuiKey_None;

void HandleTriggerBot(Memory& mem, std::uintptr_t client) {
    bool toggleState = false;

    while (true) {
        if (triggerbotEnabled) {
            bool shouldActivate = false;

            if (triggerbotMode == Always) {
                shouldActivate = true;
            }
            else if (triggerbotMode == Hold) {
                if (GetAsyncKeyState(triggerbotKey) & 0x8000) {
                    shouldActivate = true;
                }
                else {
                    shouldActivate = false;
                }
            }
            else if (triggerbotMode == Toggle) {
                if (GetAsyncKeyState(triggerbotKey) & 0x8000) {
                    toggleState = !toggleState;
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
                shouldActivate = toggleState;
            }

            if (shouldActivate) {
                uintptr_t local_player = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
                if (!local_player) continue;

                uintptr_t entity_list = mem.Read<uintptr_t>(client + offset::dwEntityList);
                if (!entity_list) continue;

                int localTeam = mem.Read<int>(local_player + offset::m_iTeamNum);

                for (int playerIndex = 1; playerIndex < 32; ++playerIndex) {
                    uintptr_t listenentry = mem.Read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
                    if (!listenentry) continue;

                    uintptr_t player = mem.Read<uintptr_t>(listenentry + 120 * (playerIndex & 0x1FF));
                    if (!player) continue;

                    int playerTeam = mem.Read<int>(player + offset::m_iTeamNum);
                    if (playerTeam == localTeam) continue;

                    int crosshair_entity_index = mem.Read<int>(local_player + offset::m_iIDEntIndex);
                    if (crosshair_entity_index < 0) continue;

                    uintptr_t listEntry = mem.Read<uintptr_t>(entity_list + 0x8 * (crosshair_entity_index >> 9) + 0x10);
                    uintptr_t entity = mem.Read<uintptr_t>(listEntry + 120 * (crosshair_entity_index & 0x1ff));
                    if (!entity) continue;

                    int entityTeam = mem.Read<int>(entity + offset::m_iTeamNum);
                    if (entityTeam == localTeam) continue;

                    int entityHealth = mem.Read<int>(entity + offset::m_iHealth);
                    if (entityHealth <= 0 || entityHealth > 100) continue;

                    std::this_thread::sleep_for(std::chrono::milliseconds(2));

                    INPUT input = { 0 };
                    input.type = INPUT_MOUSE;
                    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                    SendInput(1, &input, sizeof(INPUT));

                    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                    SendInput(1, &input, sizeof(INPUT));

                    std::this_thread::sleep_for(std::chrono::milliseconds(triggerbotDelay));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    }
}
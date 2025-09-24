#include <thread>
#include <chrono>
#include <windows.h>
#include "JumpThrow.h"

bool JumpThrow = false;
ImGuiKey JumpKey = ImGuiKey_None;

void HandleJumpThrow(Memory& mem, std::uintptr_t client) {
    bool wasKeyPressed = false;

    while (true) {
        if (JumpThrow) {
            bool shouldActivate = false;
            if (GetAsyncKeyState(JumpKey) & 0x8000) {
                shouldActivate = true;
            }

            if (shouldActivate && !wasKeyPressed) {
                // Mantener presionado el click izquierdo (M1)
                INPUT mouseInput = { 0 };
                mouseInput.type = INPUT_MOUSE;
                mouseInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &mouseInput, sizeof(INPUT));

                // Esperar 1 segundo
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                // Simular pulsación de barra espaciadora (salto)
                INPUT jumpInput = { 0 };
                jumpInput.type = INPUT_KEYBOARD;
                jumpInput.ki.wVk = VK_SPACE;
                jumpInput.ki.dwFlags = 0; // Pulsar
                SendInput(1, &jumpInput, sizeof(INPUT));

                jumpInput.ki.dwFlags = KEYEVENTF_KEYUP; // Soltar
                SendInput(1, &jumpInput, sizeof(INPUT));

                // Esperar 0,36 segundos para la cúspide del salto
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                // Soltar el click izquierdo (M1)
                mouseInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &mouseInput, sizeof(INPUT));

                wasKeyPressed = true;
            }

            // Resetear el estado cuando se suelta la tecla
            if (!(GetAsyncKeyState(JumpKey) & 0x8000)) {
                wasKeyPressed = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
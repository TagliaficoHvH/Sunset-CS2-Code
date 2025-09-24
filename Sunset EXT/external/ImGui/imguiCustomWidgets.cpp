#include "imgui.h"
#include <windows.h>
#include <unordered_map>

class KeybindWidget {
public:
    bool waitingForKey = false;
    int boundKey;
    bool* featureState;

    KeybindWidget(int defaultKey, bool* featureState) {
        // Verifica se a tecla é Mouse 4 ou Mouse 5 e substitui por uma tecla válida
        if (defaultKey == VK_XBUTTON1 || defaultKey == VK_XBUTTON2) {
            boundKey = 0x45;  // Substitui Mouse 4/Mouse 5 por "E" (tecla 0x45)
        }
        else {
            boundKey = defaultKey;
        }
        this->featureState = featureState;
    }

    // Mapeamento das teclas
    std::unordered_map<int, const char*> keyNames = {
        {VK_LBUTTON, "Left Mouse"}, {VK_RBUTTON, "Right Mouse"}, {VK_MBUTTON, "Middle Mouse"},
        {VK_XBUTTON1, "Mouse 4"}, {VK_XBUTTON2, "Mouse 5"},
        {VK_BACK, "Backspace"}, {VK_TAB, "Tab"},
        {VK_RETURN, "Enter"}, {VK_SHIFT, "Shift"}, {VK_CONTROL, "Ctrl"}, {VK_MENU, "Alt"},
        {VK_CAPITAL, "Caps Lock"}, {VK_ESCAPE, "Escape"}, {VK_SPACE, "Space"},
        {VK_PRIOR, "Page Up"}, {VK_NEXT, "Page Down"}, {VK_END, "End"}, {VK_HOME, "Home"},
        {VK_LEFT, "Left Arrow"}, {VK_UP, "Up Arrow"}, {VK_RIGHT, "Right Arrow"}, {VK_DOWN, "Down Arrow"},
        {VK_INSERT, "Insert"}, {VK_DELETE, "Delete"}, {VK_LWIN, "Left Win"}, {VK_RWIN, "Right Win"}
    };

    const char* GetKeyName(int key) {
        static char keyName[32];

        if (key == 0) return "None";

        // Verifica se a tecla está mapeada
        if (keyNames.find(key) != keyNames.end()) {
            return keyNames[key];
        }

        // Adiciona a verificação para mouse (XBUTTON1, XBUTTON2)
        if (key == VK_XBUTTON1) return "Mouse 4";
        if (key == VK_XBUTTON2) return "Mouse 5";

        // Se não estiver no mapeamento, tenta obter o nome usando o GetKeyNameTextA
        UINT scanCode = MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
        LONG lParam = scanCode << 16;
        if (GetKeyNameTextA(lParam, keyName, sizeof(keyName)) > 0) {
            return keyName;
        }

        // Se não for possível, exibe o código da tecla
        sprintf_s(keyName, "Key 0x%X", key);
        return keyName;
    }

    void HandleKeyInput() {
        static bool keyPressedLastFrame[256] = { false };

        if (waitingForKey) {
            for (int key = 1; key < 256; key++) {
                short keyState = GetAsyncKeyState(key);

                // Verifica se a tecla foi pressionada agora e não estava pressionada antes
                if ((keyState & 0x8000) && !keyPressedLastFrame[key]) {
                    // Se a tecla pressionada for Mouse 4 ou Mouse 5, não ignora mais
                    if (key == VK_XBUTTON1 || key == VK_XBUTTON2) {
                        boundKey = key;
                        waitingForKey = false;
                        return;
                    }

                    // Se for outra tecla, também realiza o binding
                    boundKey = key;
                    waitingForKey = false;
                    return;
                }

                // Armazena o estado da tecla para verificar na próxima iteração
                keyPressedLastFrame[key] = (keyState & 0x8000) != 0;
            }
        }
        else {
            if (boundKey != 0 && (GetAsyncKeyState(boundKey) & 1)) {
                *featureState = !(*featureState);
            }
        }
    }

    void Render(const char* label) {
        HandleKeyInput();

        ImGui::Checkbox(label, featureState);  // Exibe o checkbox que altera o estado da função associada
        ImGui::SameLine();

        // Exibe o botão para pressionar uma tecla e exibe o nome da tecla associada
        if (ImGui::Button(waitingForKey ? "Press a key..." : GetKeyName(boundKey))) {
            waitingForKey = true;
        }

        // Se estiver esperando uma tecla e pressionar ESC, desativa a captura de teclas
        if (waitingForKey && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            waitingForKey = false;
        }
    }
};
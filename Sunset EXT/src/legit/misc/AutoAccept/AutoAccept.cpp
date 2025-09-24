#include <thread>
#include <Windows.h>
#include <string>
#include <opencv2/opencv.hpp>
#include "AutoAccept.h"
#include "../../../cs/offsets.h"

bool AutoAccept;
bool g_inGame = false;

// Ruta al template del botón "Aceptar" (colocar accept_button.png junto al .exe)
static const std::string kAcceptButtonTemplate = "ACCEPT.png";

// Umbral de detección por matchTemplate (ajustable)
static constexpr double kMatchThreshold = 0.50;

// Captura pantalla completa y devuelve un cv::Mat BGR
cv::Mat CaptureScreen() {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenW, screenH);
    SelectObject(hMemDC, hBitmap);

    BitBlt(hMemDC, 0, 0, screenW, screenH, hScreenDC, 0, 0, SRCCOPY | CAPTUREBLT);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(bi);
    bi.biWidth = screenW;
    bi.biHeight = -screenH; // negativo para que no invierta
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    cv::Mat mat(screenH, screenW, CV_8UC3);
    GetDIBits(hMemDC, hBitmap, 0, screenH, mat.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    // Limpieza
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);

    return mat;
}

// AutoAccept thread
void AutoAcceptMatchByQueue(Memory& mem, std::uintptr_t client) {
    // Carga del template una sola vez
    cv::Mat tpl = cv::imread(kAcceptButtonTemplate, cv::IMREAD_COLOR);
    if (tpl.empty()) {
        return;
    }

    while (true) {
        // Control por checkbox (ImGui)
        if (!AutoAccept) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        // Detectar si estás dentro de una partida
        bool inGame = false;
        uintptr_t localPawn = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
        if (localPawn) {
            int team = mem.Read<int>(localPawn + offset::m_iTeamNum);   // 2 ó 3 en partida
            int health = mem.Read<int>(localPawn + offset::m_iHealth);    // 1?100 en partida

            bool teamValid = (team == 2 || team == 3);
            bool healthValid = (health > 0 && health <= 100);

            inGame = teamValid && healthValid;
        }
        g_inGame = inGame;
        if (inGame) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        // Modo detección visual cada 4 segundos
        cv::Mat screen = CaptureScreen();
        cv::Mat result;
        cv::matchTemplate(screen, tpl, result, cv::TM_CCOEFF_NORMED);

        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

        if (maxVal >= kMatchThreshold) {
            // Se detectó el botón, simula clic en centro de la plantilla
            int clickX = maxLoc.x + tpl.cols / 2;
            int clickY = maxLoc.y + tpl.rows / 2;

            SetCursorPos(clickX, clickY);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }

        // Espera antes del próximo capture
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }
}

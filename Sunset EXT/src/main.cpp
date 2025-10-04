#include <Windows.h>
#include <commdlg.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <d3d11.h>
#include <dwmapi.h>
#include <iostream>
#include <thread>
#include <windowsx.h>
#include <wininet.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <regex>
#include <sstream>
#include <curl/curl.h>

#include "../external/ImGui/ImguiCustomWidgets.cpp"
#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_dx11.h"
#include "../external/ImGui/imgui_impl_win32.h"
#include "../external/nlohmann/json.hpp"
#include "../external/lua-5.4.7/src/lua.hpp"

#include "legit/misc/NoFlash/NoFlash.h"
#include "legit/misc/Watermark/watermark.h"
#include "legit/aimbot/TriggerBot/TriggerBot.h"
#include "legit/misc/JumpThrow/JumpThrow.h"
#include "legit/misc/FakeAngles/FakeAngles.h"
#include "legit/misc/AutoAccept/AutoAccept.h"
#include "cs/bone.hpp"
#include "cs/entity.hpp"
#include "cs/vector.h"
#include "cs/view_matrix.hpp"
#include "cs/weapon_index.h"
#include "memory.h"
#include "render.h"

#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;
namespace fs = std::filesystem;

// Global Variables
bool espEnabled = false;
bool guiActive = false;
bool healthEnabled = false;
bool NameEsp = false;
bool WeaponEsp = false;
bool HealTextEsp = false;
bool skeletonEnabled = false;
bool boxEnabled = false;
bool info = false;

const char* GetWeaponName(uint64_t csPlayerPawn, Memory& mem);

// ESP Color Modes
enum ColorMode { Static, Rainbow, Pulse };
int colorModeNameEsp = Static;
int colorModeWeaponEsp = Static;
int colorModeSkeleton = Static;
int colorModeBox = Static;
int colorModeHealth = Static;
int colorModeHealTextEsp = Static;

// ESP Speed   
float Speed = 0.5f;
float SpeedNameEsp = 0.5f;
float SpeedWeaponEsp = 0.5f;
float SpeedSkeleton = 0.5f;
float SpeedBox = 0.5f;
float SpeedHealth = 0.5f;
float SpeedHealTextEsp = 0.5f;
float skeletonLineThickness = 2.0f;

RGBs white = { 1.0f, 1.0f, 1.0f };
RGBs skeletonColor = { 1.0f, 1.0f, 1.0f };
RGBs boxColor = { 1.0f, 1.0f, 1.0f };
RGBs healthColor = { 1.0f, 1.0f, 1.0f };
RGBs NameEspColor = { 1.0f, 1.0f, 1.0f };
RGBs WeaponEspColor = { 1.0f, 1.0f, 1.0f };
RGBs HealTextEspColor = { 1.0f, 1.0f, 1.0f };

// Add these global variables
ImVec4 bgColor = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.0f);
ImVec4 tabActiveColor = ImVec4(179 / 255.0f, 77 / 255.0f, 51 / 255.0f, 1.0f);
ImVec4 tabInactiveColor = ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
ImVec4 tabHoveredColor = ImVec4(65 / 255.0f, 63 / 255.0f, 62 / 255.0f, 1.0f);
ImVec4 textColor = ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f);
ImVec4 checkboxActive = ImVec4(255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f);
ImVec4 Framebc = ImVec4(51 / 255.0f, 51 / 255.0f, 51 / 255.0f, 1.0f);
ImVec4 FrameHover = ImVec4(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f);
ImVec4 buttonColor = ImVec4(255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f);
ImVec4 corcustom = ImVec4(255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);

std::vector<std::string> configFiles;

int selectedConfig = 0;
char newConfigName[64] = "";

std::string GetConfigPath() {
    char* appdataPath;
    size_t len;
    _dupenv_s(&appdataPath, &len, "APPDATA");
    if (!appdataPath) return "";

    std::string configDir = std::string(appdataPath) + "\\SunSet";
    free(appdataPath);

    if (!fs::exists(configDir)) {
        fs::create_directory(configDir);
    }

    return configDir + "\\config.json";
}

void RefreshConfigList() {
    configFiles.clear();
    std::string configDir = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/"));

    for (const auto& entry : fs::directory_iterator(configDir)) {
        if (entry.path().extension() == ".json") {
            configFiles.push_back(entry.path().filename().string());
        }
    }
}

void DeleteConfig(const std::string& configName) {
    std::string configDir = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/"));
    std::string fullPath = configDir + "\\" + configName;
    if (fs::exists(fullPath)) {
        fs::remove(fullPath);
    }
    RefreshConfigList();
}

void LoadConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (file.is_open()) {
        json config;
        file >> config;
        file.close();

        espEnabled = config.value("espEnabled", false);
        healthEnabled = config.value("healthEnabled", false);
        NameEsp = config.value("NameEsp", false);
        WeaponEsp = config.value("WeaponEsp", false);
        HealTextEsp = config.value("HealTextEsp", false);
        skeletonEnabled = config.value("skeletonEnabled", false);
        boxEnabled = config.value("boxEnabled", false);
        watermarkEnabled = config.value("watermarkEnabled", false);
        NoFlash = config.value("NoFlash", false);
        fakeAngles = config.value("fakeAngles", false);
		JumpThrow = config.value("JumpThrow", false);
		JumpKey = config.value("JumpKey", ImGuiKey_None);
		AutoAccept = config.value("AutoAccept", false);
        triggerbotEnabled = config.value("triggerbotEnabled", false);
        triggerbotDelay = config.value("triggerbotDelay", 2);
        triggerbotMode = static_cast<TriggerbotMode>(config.value("triggerbotMode", Always));
        triggerbotKey = config.value("triggerbotKey", ImGuiKey_None);
        watermarkTransparency = config.value("watermarkTransparency", 0.5f);

        colorModeNameEsp = config.value("colorModeNameEsp", Static);
        colorModeWeaponEsp = config.value("colorModeWeaponEsp", Static);
        colorModeSkeleton = config.value("colorModeSkeleton", Static);
        colorModeBox = config.value("colorModeBox", Static);
        colorModeHealth = config.value("colorModeHealth", Static);
        colorModeHealTextEsp = config.value("colorModeHealTextEsp", Static);

        SpeedNameEsp = config.value("SpeedNameEsp", 0.5f);
        SpeedWeaponEsp = config.value("SpeedWeaponEsp", 0.5f);
        SpeedSkeleton = config.value("SpeedSkeleton", 0.5f);
        SpeedBox = config.value("SpeedBox", 0.5f);
        SpeedHealth = config.value("SpeedHealth", 0.5f);
        SpeedHealTextEsp = config.value("SpeedHealTextEsp", 0.5f);

        skeletonLineThickness = config.value("skeletonLineThickness", 2.0f);

        auto skeletonColorArray = config.value("skeletonColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        skeletonColor = { skeletonColorArray[0], skeletonColorArray[1], skeletonColorArray[2] };

        auto boxColorArray = config.value("boxColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        boxColor = { boxColorArray[0], boxColorArray[1], boxColorArray[2] };

        auto healthColorArray = config.value("healthColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        healthColor = { healthColorArray[0], healthColorArray[1], healthColorArray[2] };

        auto NameEspColorArray = config.value("NameEspColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        NameEspColor = { NameEspColorArray[0], NameEspColorArray[1], NameEspColorArray[2] };

        auto WeaponEspColorArray = config.value("WeaponEspColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        WeaponEspColor = { WeaponEspColorArray[0], WeaponEspColorArray[1], WeaponEspColorArray[2] };

        auto HealTextEspColorArray = config.value("HealTextEspColor", std::vector<float>{1.0f, 1.0f, 1.0f});
        HealTextEspColor = { HealTextEspColorArray[0], HealTextEspColorArray[1], HealTextEspColorArray[2] };

        auto bgColorArray = config.value("bgColor", std::vector<float>{25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.0f});
        bgColor = { bgColorArray[0], bgColorArray[1], bgColorArray[2], bgColorArray[3] };

        auto tabActiveColorArray = config.value("tabActiveColor", std::vector<float>{179 / 255.0f, 77 / 255.0f, 51 / 255.0f, 1.0f});
        tabActiveColor = { tabActiveColorArray[0], tabActiveColorArray[1], tabActiveColorArray[2], tabActiveColorArray[3] };

        auto tabInactiveColorArray = config.value("tabInactiveColor", std::vector<float>{0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f});
        tabInactiveColor = { tabInactiveColorArray[0], tabInactiveColorArray[1], tabInactiveColorArray[2], tabInactiveColorArray[3] };

        auto tabHoveredColorArray = config.value("tabHoveredColor", std::vector<float>{65 / 255.0f, 63 / 255.0f, 62 / 255.0f, 1.0f});
        tabHoveredColor = { tabHoveredColorArray[0], tabHoveredColorArray[1], tabHoveredColorArray[2], tabHoveredColorArray[3] };

        auto textColorArray = config.value("textColor", std::vector<float>{255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f});
        textColor = { textColorArray[0], textColorArray[1], textColorArray[2], textColorArray[3] };

        auto checkboxActiveArray = config.value("checkboxActive", std::vector<float>{255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f});
        checkboxActive = { checkboxActiveArray[0], checkboxActiveArray[1], checkboxActiveArray[2], checkboxActiveArray[3] };

        auto FramebcArray = config.value("Framebc", std::vector<float>{51 / 255.0f, 51 / 255.0f, 51 / 255.0f, 1.0f});
        Framebc = { FramebcArray[0], FramebcArray[1], FramebcArray[2], FramebcArray[3] };

        auto FrameHoverArray = config.value("FrameHover", std::vector<float>{128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f});
        FrameHover = { FrameHoverArray[0], FrameHoverArray[1], FrameHoverArray[2], FrameHoverArray[3] };

        auto buttonColorArray = config.value("buttonColor", std::vector<float>{255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f});
        buttonColor = { buttonColorArray[0], buttonColorArray[1], buttonColorArray[2], buttonColorArray[3] };

        auto watermarkTextColorArray = config.value("watermarkTextColor", std::vector<float>{255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f});
        watermarkTextColor = { watermarkTextColorArray[0], watermarkTextColorArray[1], watermarkTextColorArray[2], watermarkTextColorArray[3] };

        auto watermarkBackGroundColorArray = config.value("watermarkBackGroundColor", std::vector<float>{0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f});
        watermarkBackGroundColor = { watermarkBackGroundColorArray[0], watermarkBackGroundColorArray[1], watermarkBackGroundColorArray[2], watermarkBackGroundColorArray[3] };

        auto corcustomArray = config.value("corcustom", std::vector<float>{255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f});
        corcustom = { corcustomArray[0], corcustomArray[1], corcustomArray[2], corcustomArray[3] };
    }
}

void SaveConfig(const std::string& filePath) {
    std::ofstream outFile(filePath);
    if (outFile.is_open()) {
        json config;
        config["espEnabled"] = espEnabled;
        config["healthEnabled"] = healthEnabled;
        config["NameEsp"] = NameEsp;
        config["WeaponEsp"] = WeaponEsp;
        config["HealTextEsp"] = HealTextEsp;
        config["skeletonEnabled"] = skeletonEnabled;
        config["boxEnabled"] = boxEnabled;
        config["watermarkEnabled"] = watermarkEnabled;
        config["NoFlash"] = NoFlash;
		config["fakeAngles"] = fakeAngles;
		config["JumpThrow"] = JumpThrow;
		config["JumpKey"] = JumpKey;
		config["AutoAccept"] = AutoAccept;
        config["triggerbotEnabled"] = triggerbotEnabled;
        config["triggerbotDelay"] = triggerbotDelay;
        config["triggerbotMode"] = triggerbotMode;
        config["triggerbotKey"] = triggerbotKey;
        config["watermarkTransparency"] = watermarkTransparency;

        config["colorModeNameEsp"] = colorModeNameEsp;
        config["colorModeWeaponEsp"] = colorModeWeaponEsp;
        config["colorModeSkeleton"] = colorModeSkeleton;
        config["colorModeBox"] = colorModeBox;
        config["colorModeHealth"] = colorModeHealth;
        config["colorModeHealTextEsp"] = colorModeHealTextEsp;

        config["SpeedNameEsp"] = SpeedNameEsp;
        config["SpeedWeaponEsp"] = SpeedWeaponEsp;
        config["SpeedSkeleton"] = SpeedSkeleton;
        config["SpeedBox"] = SpeedBox;
        config["SpeedHealth"] = SpeedHealth;
        config["SpeedHealTextEsp"] = SpeedHealTextEsp;

        config["skeletonLineThickness"] = skeletonLineThickness;

        config["skeletonColor"] = { skeletonColor.r, skeletonColor.g, skeletonColor.b };
        config["boxColor"] = { boxColor.r, boxColor.g, boxColor.b };
        config["healthColor"] = { healthColor.r, healthColor.g, healthColor.b };
        config["NameEspColor"] = { NameEspColor.r, NameEspColor.g, NameEspColor.b };
        config["WeaponEspColor"] = { WeaponEspColor.r, WeaponEspColor.g, WeaponEspColor.b };
        config["HealTextEspColor"] = { HealTextEspColor.r, HealTextEspColor.g, HealTextEspColor.b };

        config["bgColor"] = { bgColor.x, bgColor.y, bgColor.z, bgColor.w };
        config["tabActiveColor"] = { tabActiveColor.x, tabActiveColor.y, tabActiveColor.z, tabActiveColor.w };
        config["tabInactiveColor"] = { tabInactiveColor.x, tabInactiveColor.y, tabInactiveColor.z, tabInactiveColor.w };
        config["tabHoveredColor"] = { tabHoveredColor.x, tabHoveredColor.y, tabHoveredColor.z, tabHoveredColor.w };
        config["textColor"] = { textColor.x, textColor.y, textColor.z, textColor.w };
        config["checkboxActive"] = { checkboxActive.x, checkboxActive.y, checkboxActive.z, checkboxActive.w };
        config["Framebc"] = { Framebc.x, Framebc.y, Framebc.z, Framebc.w };
        config["FrameHover"] = { FrameHover.x, FrameHover.y, FrameHover.z, FrameHover.w };
        config["buttonColor"] = { buttonColor.x, buttonColor.y, buttonColor.z, buttonColor.w };
        config["watermarkTextColor"] = { watermarkTextColor.x, watermarkTextColor.y, watermarkTextColor.z, watermarkTextColor.w };
        config["watermarkBackGroundColor"] = { watermarkBackGroundColor.x, watermarkBackGroundColor.y, watermarkBackGroundColor.z, watermarkBackGroundColor.w };
        config["corcustom"] = { corcustom.x, corcustom.y, corcustom.z, corcustom.w };

        outFile << config.dump(4);
        outFile.close();
        RefreshConfigList();
    }
}

// Función callback para libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Descarga el contenido de una URL en un string
std::string downloadFile(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "OffsetUpdater/1.0");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error descargando " << url << ": "
                << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

void extractOffsets(const std::string& content,
    const std::vector<std::string>& names,
    std::unordered_map<std::string, std::string>& out)
{
    for (const auto& name : names) {
        std::regex pattern;

        if (name == "m_Item") {
            // Solo el m_Item de C_AttributeContainer
            pattern = std::regex(
                R"(public static class C_AttributeContainer[\s\S]*?public const nint m_Item\s*=\s*(0x[0-9A-Fa-f]+);)"
            );
        }
        else if (name == "m_AttributeManager") {
            // Solo el m_AttributeManager de C_EconEntity
            pattern = std::regex(
                R"(public static class C_EconEntity[\s\S]*?public const nint m_AttributeManager\s*=\s*(0x[0-9A-Fa-f]+);)"
            );
        }
        else if (name == "m_entitySpottedState") {
            // Solo el m_entitySpottedState de C_CSPlayerPawn
            pattern = std::regex(
                R"(public static class C_CSPlayerPawn[\s\S]*?public const nint m_entitySpottedState\s*=\s*(0x[0-9A-Fa-f]+);)"
            );
		}
        else if (name == "m_fFlags") {
            // Solo el m_fFlags de C_BaseEntity
            pattern = std::regex(
                R"(public static class C_BaseEntity[\s\S]*?public const nint m_fFlags\s*=\s*(0x[0-9A-Fa-f]+);)"
            );
		}
        else {
            // General: busca todas las apariciones y se queda con la última
            pattern = std::regex(
                "public const nint\\s+" + name + R"(\s*=\s*(0x[0-9A-Fa-f]+);)"
            );
        }

        // --- búsqueda ---
        std::sregex_iterator iter(content.begin(), content.end(), pattern);
        std::sregex_iterator end;

        std::string lastMatch;
        for (; iter != end; ++iter) {
            lastMatch = (*iter)[1].str();
        }

        if (!lastMatch.empty()) {
            out[name] = lastMatch;
        }
        else {
            std::cerr << "[-] No se encontró offset: " << name << std::endl;
        }
    }
}

void updateOffsets() {
    // URLs RAW
    std::string urlClientDll = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.cs";
    std::string urlOffsets = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.cs";

    // Offsets que queremos extraer
    std::vector<std::string> clientDllOffsets = {
        "m_flFlashBangTime", "m_iIDEntIndex", "m_fFlags", "v_angle", "m_iHealth", "m_iTeamNum",
        "m_vOldOrigin", "m_hPlayerPawn", "m_pGameSceneNode", "m_modelState",
        "m_entitySpottedState", "m_pPawnSubclass", "m_sSanitizedPlayerName", "m_pClippingWeapon",
        "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex", "m_iAccount",
        "m_hPawn", "m_bIsLocalPlayerController", "m_iPing"
    };

    std::vector<std::string> offsetsCs = {
        "dwViewMatrix", "dwLocalPlayerPawn", "dwEntityList", "dwLocalPlayerController", "dwViewAngles"
    };

    // Descargar los archivos
    std::string clientDllContent = downloadFile(urlClientDll);
    std::string offsetsContent = downloadFile(urlOffsets);

    // Extraer valores
    std::unordered_map<std::string, std::string> result;
    extractOffsets(clientDllContent, clientDllOffsets, result);
    extractOffsets(offsetsContent, offsetsCs, result);

    // Convertir a JSON
    json j;
    for (const auto& [key, val] : result) {
        j[key] = val;
    }

    // Guardar en offsets.json
    std::ofstream outFile("offsets.json");
    outFile << j.dump(4); // pretty print
    outFile.close();

    std::cout << "[+] Offsets guardados en offsets.json\n";
    offset::loadFromJson(result);
}

namespace offset {
    void loadFromJson(const std::unordered_map<std::string, std::string>& data) {
        auto hexToPtrdiff = [](const std::string& hexStr) -> std::ptrdiff_t {
            std::stringstream ss;
            ss << std::hex << hexStr;
            std::ptrdiff_t val;
            ss >> val;
            return val;
            };

        if (data.count("dwViewMatrix")) dwViewMatrix = hexToPtrdiff(data.at("dwViewMatrix"));
        if (data.count("dwLocalPlayerPawn")) dwLocalPlayerPawn = hexToPtrdiff(data.at("dwLocalPlayerPawn"));
        if (data.count("dwEntityList")) dwEntityList = hexToPtrdiff(data.at("dwEntityList"));
        if (data.count("dwLocalPlayerController")) dwLocalPlayerController = hexToPtrdiff(data.at("dwLocalPlayerController"));
        if (data.count("dwViewAngles")) dwViewAngles = hexToPtrdiff(data.at("dwViewAngles"));

        if (data.count("m_flFlashBangTime")) m_flFlashBangTime = hexToPtrdiff(data.at("m_flFlashBangTime"));
        if (data.count("m_iIDEntIndex")) m_iIDEntIndex = hexToPtrdiff(data.at("m_iIDEntIndex"));
        if (data.count("m_fFlags")) m_fFlags = hexToPtrdiff(data.at("m_fFlags"));
        if (data.count("v_angle")) v_angle = hexToPtrdiff(data.at("v_angle"));
        if (data.count("m_iHealth")) m_iHealth = hexToPtrdiff(data.at("m_iHealth"));
        if (data.count("m_iTeamNum")) m_iTeamNum = hexToPtrdiff(data.at("m_iTeamNum"));
        if (data.count("m_vOldOrigin")) m_vOldOrigin = hexToPtrdiff(data.at("m_vOldOrigin"));
        if (data.count("m_hPlayerPawn")) m_hPlayerPawn = hexToPtrdiff(data.at("m_hPlayerPawn"));

        if (data.count("m_pGameSceneNode")) m_pGameSceneNode = hexToPtrdiff(data.at("m_pGameSceneNode"));
        if (data.count("m_modelState")) m_modelState = hexToPtrdiff(data.at("m_modelState"));
        if (data.count("m_entitySpottedState")) m_entitySpottedState = hexToPtrdiff(data.at("m_entitySpottedState"));
        if (data.count("m_pPawnSubclass")) m_pPawnSubclass = hexToPtrdiff(data.at("m_pPawnSubclass"));

        if (data.count("m_sSanitizedPlayerName")) m_sSanitizedPlayerName = hexToPtrdiff(data.at("m_sSanitizedPlayerName"));
        if (data.count("m_pClippingWeapon")) m_pClippingWeapon = hexToPtrdiff(data.at("m_pClippingWeapon"));
        if (data.count("m_AttributeManager")) m_AttributeManager = hexToPtrdiff(data.at("m_AttributeManager"));
        if (data.count("m_Item")) m_Item = hexToPtrdiff(data.at("m_Item"));
        if (data.count("m_iItemDefinitionIndex")) m_iItemDefinitionIndex = hexToPtrdiff(data.at("m_iItemDefinitionIndex"));
        if (data.count("m_iAccount")) m_iAccount = hexToPtrdiff(data.at("m_iAccount"));
        if (data.count("m_hPawn")) m_hPawn = hexToPtrdiff(data.at("m_hPawn"));
        if (data.count("m_bIsLocalPlayerController")) m_bIsLocalPlayerController = hexToPtrdiff(data.at("m_bIsLocalPlayerController"));
        if (data.count("m_iPing")) m_iPing = hexToPtrdiff(data.at("m_iPing"));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lua API Section
// This works but it needs to add documentation for the variables, that is, create the cheat API so it can be used in Luas. 
// I don't think I'll make the API because it's not a very well-known cheat, but if you're going to copy this, make the API so it works.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Global Lua variable
lua_State* L;

void InitLua() {
    L = luaL_newstate();
    luaL_openlibs(L);
}

void CloseLua() {
    lua_close(L);
}

int Lua_OpenFolder(lua_State* L) {
    const char* folderPath = luaL_checkstring(L, 1);
    ShellExecuteA(NULL, "open", folderPath, NULL, NULL, SW_SHOWDEFAULT);
    return 0;
}

void ExecuteLuaScript(const std::string& script) {
    if (luaL_dofile(L, script.c_str()) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Lua error: " << error << std::endl;
        lua_pop(L, 1);
    }
}

void UnloadLuaScript() {
    lua_close(L);
    InitLua();
}

std::vector<std::string> luaScripts;

std::string GetLuaScriptsPath() {
    char* appdataPath;
    size_t len;
    _dupenv_s(&appdataPath, &len, "APPDATA");
    if (!appdataPath) return "";

    std::string luaDir = std::string(appdataPath) + "\\SunSet\\Lua";
    free(appdataPath);

    if (!fs::exists(luaDir)) {
        fs::create_directory(luaDir);
    }

    return luaDir;
}

void RefreshLuaScriptsList() {
    luaScripts.clear();
    std::string luaDir = GetLuaScriptsPath();

    for (const auto& entry : fs::directory_iterator(luaDir)) {
        if (entry.path().extension() == ".lua") {
            luaScripts.push_back(entry.path().filename().string());
        }
    }
}

void DeleteLuaScript(const std::string& scriptName) {
    std::string luaDir = GetLuaScriptsPath();
    std::string fullPath = luaDir + "\\" + scriptName;
    if (fs::exists(fullPath)) {
        fs::remove(fullPath);
    }
    RefreshLuaScriptsList();
}

void CreateLuaScript(const std::string& scriptName) {
    std::string luaDir = GetLuaScriptsPath();
    std::string fullPath = luaDir + "\\" + scriptName + ".lua";
    std::ofstream outFile(fullPath);
    if (outFile.is_open()) {
        outFile << "-- New Lua Script\n";
        outFile.close();
        RefreshLuaScriptsList();
    }
}

void EditLuaScript(const std::string& scriptName) {
    std::string luaDir = GetLuaScriptsPath();
    std::string fullPath = luaDir + "\\" + scriptName;
    ShellExecuteA(NULL, "open", fullPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShowTriggerbotContextMenu() {
    if (ImGui::BeginPopup("TriggerbotContextMenu")) {
        if (ImGui::MenuItem("Always", nullptr, triggerbotMode == Always)) {
            triggerbotMode = Always;
        }
        if (ImGui::MenuItem("Hold", nullptr, triggerbotMode == Hold)) {
            triggerbotMode = Hold;
        }
        if (ImGui::MenuItem("Toggle", nullptr, triggerbotMode == Toggle)) {
            triggerbotMode = Toggle;
        }
        ImGui::EndPopup();
    }
}

void debugLog(const std::string& message) {
    OutputDebugStringA((message + "\n").c_str());
}

ImVec4 GetRainbowColor(float speed) {
    float time = ImGui::GetTime() * speed;
    return ImVec4(
        0.5f + 0.5f * sin(time + 0.0f),
        0.5f + 0.5f * sin(time + 2.0f),
        0.5f + 0.5f * sin(time + 4.0f),
        1.0f
    );
}

ImVec4 GetPulseColor(float speed, const ImVec4& baseColor) {
    float time = ImGui::GetTime() * speed;
    float pulse = (sin(time) + 1.0f) / 2.0f;
    return ImVec4(
        baseColor.x * pulse,
        baseColor.y * pulse,
        baseColor.z * pulse,
        baseColor.w
    );
}

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

extern Memory mem;
extern std::uintptr_t client;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {

    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return 0L;
    }

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0L;
    }
    switch (message)
    {
    case WM_NCHITTEST:
    {
        const LONG borderWidth = GetSystemMetrics(SM_CXSIZEFRAME);
        const LONG titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
        POINT curserPos = { GET_X_LPARAM(w_param), GET_Y_LPARAM(l_param) };
        RECT windowRect;
        GetWindowRect(window, &windowRect);

        if (curserPos.y >= windowRect.top && curserPos.y < windowRect.top + titleBarHeight)
            return HTCAPTION;

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(window, message, w_param, l_param);
}


void ToggleWindowTransparency(HWND hwnd, bool transparent) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (transparent) {
        style |= WS_EX_TRANSPARENT;
    }
    else {
        style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLong(hwnd, GWL_EXSTYLE, style);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
}

std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();

int frameCount = 0;
int fps = 0;

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"S";

    RegisterClassExW(&wc);

    const HWND overlay = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName,
        L"S",
        WS_POPUP,
        0,
        0,
        screenWidth,
        screenHeight,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(overlay, &client_area);

        RECT window_area{};
        GetWindowRect(overlay, &window_area);

        POINT diff{};
        ClientToScreen(overlay, &diff);

        const MARGINS margins{
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom,
        };

        DwmExtendFrameIntoClientArea(overlay, &margins);
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 144U;
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = overlay;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0U,
        levels,
        2U,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        &level,
        &device_context
    );

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer) {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else
        return 1;

    ShowWindow(overlay, cmd_show);
    UpdateWindow(overlay);

    ImGui::CreateContext();
    ImGui::StyleColorsDark(); // Tema moderno


    ImGui_ImplWin32_Init(overlay);
    ImGui_ImplDX11_Init(device, device_context);

    // Añadir fuente personalizada
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Roboto-Regular.ttf", 18.0f);

    InitLua();
    updateOffsets();

    bool running = true;

    // NoFlash
    std::thread noFlashThread(HandleNoFlash, std::ref(mem), client);
    noFlashThread.detach();

    // TriggerBot
    std::thread triggerbotthread(HandleTriggerBot, std::ref(mem), client);
    triggerbotthread.detach();

	// JumpThrow
	std::thread jumpThrowThread(HandleJumpThrow, std::ref(mem), client);
	jumpThrowThread.detach();

    // FakeAngles
    std::thread fakeAnglesThread(HandleFakeAngles, std::ref(mem), client);
    fakeAnglesThread.detach();

    // AutoAccept
    std::thread autoAcceptThread(AutoAcceptMatchByQueue, std::ref(mem), client);
    autoAcceptThread.detach();


    while (running)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running = false;
            }
        }

        if ((GetAsyncKeyState(VK_INSERT) & 1) || (GetAsyncKeyState(VK_DELETE) & 1)) {
            guiActive = !guiActive;
            ImGui::GetIO().MouseDrawCursor = guiActive;
            ToggleWindowTransparency(overlay, !guiActive);
        }

        if (!running)
            break;

        // FPS calculation
        frameCount++;

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - lastTime;
        if (elapsedTime.count() >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }

        uintptr_t localPlayer = mem.Read<uintptr_t>(client + offset::dwLocalPlayerPawn);
        Vector3 Localorigin = mem.Read<uintptr_t>(localPlayer + offset::m_vOldOrigin);
        view_matrix_t view_matrix = mem.Read<view_matrix_t>(client + offset::dwViewMatrix);
        int localTeam = mem.Read<int>(localPlayer + offset::m_iTeamNum);
        uintptr_t entity_list = mem.Read<uintptr_t>(client + offset::dwEntityList);
        uintptr_t localPlayerController = mem.Read<uintptr_t>(client + offset::dwLocalPlayerController);
        int ping = mem.Read<int>(localPlayerController + offset::m_iPing);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (guiActive) {
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_WindowBg] = bgColor;
            style.Colors[ImGuiCol_TabActive] = tabActiveColor;
            style.Colors[ImGuiCol_Tab] = tabInactiveColor;
            style.Colors[ImGuiCol_TabHovered] = tabHoveredColor;
            style.Colors[ImGuiCol_Text] = textColor;
            style.Colors[ImGuiCol_CheckMark] = checkboxActive;
            style.Colors[ImGuiCol_FrameBg] = Framebc;
            style.Colors[ImGuiCol_FrameBgHovered] = FrameHover;
            style.Colors[ImGuiCol_Button] = buttonColor;
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(buttonColor.x + 0.1f, buttonColor.y + 0.1f, buttonColor.z + 0.1f, buttonColor.w);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(buttonColor.x - 0.1f, buttonColor.y - 0.1f, buttonColor.z - 0.1f, buttonColor.w);
            style.WindowRounding = 12.0f;
            style.FrameRounding = 8.0f;
            style.ScrollbarRounding = 8.0f;
            style.GrabRounding = 8.0f;
            style.FramePadding = ImVec2(12, 8);
            style.ItemSpacing = ImVec2(10, 8);
            style.WindowPadding = ImVec2(16, 12);
            style.Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 0.6f);
            style.WindowBorderSize = 2.0f;
            style.Colors[ImGuiCol_WindowBg].w = 0.90f; // 90% opacidad

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 10));

            ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::Begin("Sunset", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            static int activeTab = 0;

            if (ImGui::BeginTabBar("MainTabsBar")) {
                if (ImGui::BeginTabItem("Aimbot")) {
                    // Contenido de la pestaña Aimbot
                    ImGui::Checkbox("Enable Triggerbot", &triggerbotEnabled);
                    ImGui::SliderInt("Triggerbot Delay", &triggerbotDelay, 1, 200);

                    ImGui::Text("Triggerbot Keybind:");
                    ImGui::SameLine();
                    ImGui::PushID("TriggerbotKeybind");
                    if (ImGui::Button(triggerbotKey != ImGuiKey_None ? ImGui::GetKeyName(triggerbotKey) : "Set Key")) {
                        ImGui::OpenPopup("SetTriggerbotKey");
                    }
                    if (ImGui::BeginPopup("SetTriggerbotKey")) {
                        ImGui::Text("Press any key...");
                        for (int key = 0x01; key <= 0xFE; key++) {
                            if (GetAsyncKeyState(key) & 0x8000) {
                                triggerbotKey = static_cast<ImGuiKey>(key);
                                ImGui::CloseCurrentPopup();
                            }
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();

                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("TriggerbotContextMenu");
                    }
                    ShowTriggerbotContextMenu();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Visuals")) {
                    // Contenido de la pestaña Visuals
                    ImGui::Checkbox("Enable ESP", &espEnabled);

                    ImGui::Checkbox("NameESP", &NameEsp);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("NameESP Color", ImVec4(NameEspColor.r, NameEspColor.g, NameEspColor.b, 1.0f))) {
                        ImGui::OpenPopup("NameESPColorPicker");
                    }
                    if (ImGui::BeginPopup("NameESPColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&NameEspColor);
                        ImGui::SliderFloat("Speed", &SpeedNameEsp, 0.1f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeNameEsp, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("WeaponESP", &WeaponEsp);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("WeaponESP Color", ImVec4(WeaponEspColor.r, WeaponEspColor.g, WeaponEspColor.b, 1.0f))) {
                        ImGui::OpenPopup("WeaponESPColorPicker");
                    }
                    if (ImGui::BeginPopup("WeaponESPColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&WeaponEspColor);
                        ImGui::SliderFloat("Speed", &SpeedWeaponEsp, 0.1f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeWeaponEsp, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("Skeleton", &skeletonEnabled);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("Skeleton Color", ImVec4(skeletonColor.r, skeletonColor.g, skeletonColor.b, 1.0f))) {
                        ImGui::OpenPopup("SkeletonColorPicker");
                    }
                    if (ImGui::BeginPopup("SkeletonColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&skeletonColor);
                        ImGui::SliderFloat("Speed", &SpeedSkeleton, 0.1f, 5.0f, "%.1f");
                        ImGui::SliderFloat("Line Thickness", &skeletonLineThickness, 1.0f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeSkeleton, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("Box", &boxEnabled);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("Box Color", ImVec4(boxColor.r, boxColor.g, boxColor.b, 1.0f))) {
                        ImGui::OpenPopup("BoxColorPicker");
                    }
                    if (ImGui::BeginPopup("BoxColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&boxColor);
                        ImGui::SliderFloat("Speed", &SpeedBox, 0.1f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeBox, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("HealthBar", &healthEnabled);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("Health Color", ImVec4(healthColor.r, healthColor.g, healthColor.b, 1.0f))) {
                        ImGui::OpenPopup("HealthColorPicker");
                    }
                    if (ImGui::BeginPopup("HealthColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&healthColor);
                        ImGui::SliderFloat("Speed", &SpeedHealth, 0.1f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeHealth, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }

                    ImGui::Checkbox("HealTextESP", &HealTextEsp);
                    ImGui::SameLine(140);
                    if (ImGui::ColorButton("HealTextESP Color", ImVec4(HealTextEspColor.r, HealTextEspColor.g, HealTextEspColor.b, 1.0f))) {
                        ImGui::OpenPopup("HealTextESPColorPicker");
                    }
                    if (ImGui::BeginPopup("HealTextESPColorPicker")) {
                        ImGui::ColorPicker3("##picker", (float*)&HealTextEspColor);
                        ImGui::SliderFloat("Speed", &SpeedHealTextEsp, 0.1f, 5.0f, "%.1f");
                        static const char* colorModes[] = { "Static", "Rainbow", "Pulse" };
                        ImGui::Combo("Color Mode", &colorModeHealTextEsp, colorModes, IM_ARRAYSIZE(colorModes));
                        ImGui::EndPopup();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc")) {
                    // Contenido de la pestaña Misc
                    ImGui::Checkbox("Watermark", &watermarkEnabled);
                    ImGui::Checkbox("Auto Accept", &AutoAccept);
                    ImGui::SameLine();
                    const char* estadoTexto = "OFF";
                    ImVec4 colorEstado = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // gris por defecto

                    if (AutoAccept) {
                        if (g_inGame) {
                            estadoTexto = "In game (paused)";
                            colorEstado = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);  // rojo suave
                        }
                        else {
                            estadoTexto = "ON";
                            colorEstado = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);  // verde
                        }
                    }

                    ImGui::TextColored(colorEstado, "State: %s", estadoTexto);
                    ImGui::Checkbox("No Flash", &NoFlash);
					ImGui::Checkbox("Fake Angles", &fakeAngles);
                    ImGui::Checkbox("JumpThrow", &JumpThrow);
                    ImGui::Text("JumpThrow Keybind:");
                    ImGui::SameLine();
                    ImGui::PushID("JumpKey");
                    if (ImGui::Button(JumpKey != ImGuiKey_None ? ImGui::GetKeyName(JumpKey) : "Set Key")) {
                        ImGui::OpenPopup("SetJumpThrowKey");
                    }
                    if (ImGui::BeginPopup("SetJumpThrowKey")) {
                        ImGui::Text("Press any key...");
                        for (int key = 0x01; key <= 0xFE; key++) {
                            if (GetAsyncKeyState(key) & 0x8000) {
                                JumpKey = static_cast<ImGuiKey>(key);
                                ImGui::CloseCurrentPopup();
                            }
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Configs")) {
                    ImGui::Spacing();
                    ImGui::Text("Available Configs:");
                    ImGui::Spacing();

                    if (ImGui::BeginListBox("##ConfigList", ImVec2(300, 150))) {
                        for (size_t i = 0; i < configFiles.size(); ++i) {
                            bool isSelected = (selectedConfig == i);
                            if (ImGui::Selectable(configFiles[i].c_str(), isSelected)) {
                                selectedConfig = i;
                            }
                        }
                        ImGui::EndListBox();
                    }

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Button("Load Selected", ImVec2(150, 40))) {
                        if (!configFiles.empty()) {
                            std::string selectedFile = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/")) + "\\" + configFiles[selectedConfig];
                            LoadConfig(selectedFile);
                        }
                    }

                    if (ImGui::Button("Save Selected", ImVec2(150, 40))) {
                        if (!configFiles.empty()) {
                            std::string selectedFile = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/")) + "\\" + configFiles[selectedConfig];
                            SaveConfig(selectedFile);
                        }
                    }

                    if (ImGui::Button("Refresh Configs", ImVec2(150, 40))) {
                        RefreshConfigList();
                    }
                    ImGui::EndGroup();


                    ImGui::Spacing();

                    ImGui::SameLine();
                    if (ImGui::Button("Delete Selected", ImVec2(130, 40))) {
                        if (!configFiles.empty()) {
                            DeleteConfig(configFiles[selectedConfig]);
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Reset To Default", ImVec2(140, 40))) {
                        if (!configFiles.empty()) {
                            espEnabled = false;
                            healthEnabled = false;
                            NameEsp = false;
                            WeaponEsp = false;
                            HealTextEsp = false;
                            skeletonEnabled = false;
                            boxEnabled = false;
                            watermarkEnabled = false;
                            NoFlash = false;
							fakeAngles = false;
                            JumpThrow = false;
                            JumpKey = ImGuiKey_None;
							AutoAccept = false;
                            triggerbotEnabled = false;
                            triggerbotDelay = 2;
                            triggerbotMode = Always;
                            triggerbotKey = ImGuiKey_None;
                            watermarkTransparency = 0.5f;

                            colorModeNameEsp = Static;
                            colorModeWeaponEsp = Static;
                            colorModeSkeleton = Static;
                            colorModeBox = Static;
                            colorModeHealth = Static;
                            colorModeHealTextEsp = Static;

                            SpeedNameEsp = 0.5f;
                            SpeedWeaponEsp = 0.5f;
                            SpeedSkeleton = 0.5f;
                            SpeedBox = 0.5f;
                            SpeedHealth = 0.5f;
                            SpeedHealTextEsp = 0.5f;

                            skeletonLineThickness = 2.0f;

                            skeletonColor = { 1.0f, 1.0f, 1.0f };
                            boxColor = { 1.0f, 1.0f, 1.0f };
                            healthColor = { 1.0f, 1.0f, 1.0f };
                            NameEspColor = { 1.0f, 1.0f, 1.0f };
                            WeaponEspColor = { 1.0f, 1.0f, 1.0f };
                            HealTextEspColor = { 1.0f, 1.0f, 1.0f };

                            bgColor = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.0f);
                            tabActiveColor = ImVec4(179 / 255.0f, 77 / 255.0f, 51 / 255.0f, 1.0f);
                            tabInactiveColor = ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
                            tabHoveredColor = ImVec4(65 / 255.0f, 63 / 255.0f, 62 / 255.0f, 1.0f);
                            textColor = ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f);
                            checkboxActive = ImVec4(255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f);
                            Framebc = ImVec4(51 / 255.0f, 51 / 255.0f, 51 / 255.0f, 1.0f);
                            FrameHover = ImVec4(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f);
                            buttonColor = ImVec4(255 / 255.0f, 165 / 255.0f, 0 / 255.0f, 1.0f);
                            watermarkTextColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                            watermarkBackGroundColor = ImVec4(25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f);
                            corcustom = ImVec4(255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);

                            std::string selectedFile = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/")) + "\\" + configFiles[selectedConfig];
                            SaveConfig(selectedFile);
                        }
                    }


                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Save New Config:");
                    ImGui::InputText("##NewConfig", newConfigName, IM_ARRAYSIZE(newConfigName));

                    ImGui::SameLine();                    ImGui::SameLine();
                    if (ImGui::Button("Save As", ImVec2(100, 40))) {
                        if (strlen(newConfigName) > 0) {
                            std::string newConfigPath = GetConfigPath().substr(0, GetConfigPath().find_last_of("\\/")) + "\\" + std::string(newConfigName) + ".json";
                            SaveConfig(newConfigPath);
                        }
                    }
                    ImGui::EndTabItem();
                }


                if (ImGui::BeginTabItem("Luas")) {
                    ImGui::Text("Available Lua Scripts:");
                    ImGui::Spacing();

                    static bool firstOpen = true;
                    if (firstOpen) {
                        RefreshLuaScriptsList();
                        firstOpen = false;
                    }

                    if (ImGui::BeginListBox("##LuaScriptList", ImVec2(300, 150))) {
                        for (size_t i = 0; i < luaScripts.size(); ++i) {
                            bool isSelected = (selectedConfig == i);
                            if (ImGui::Selectable(luaScripts[i].c_str(), isSelected)) {
                                selectedConfig = i;
                            }
                        }
                        ImGui::EndListBox();
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Columns(2, nullptr, false);

                    if (ImGui::Button("Load Selected", ImVec2(-1, 40))) {
                        if (!luaScripts.empty()) {
                            std::string selectedFile = GetLuaScriptsPath() + "\\" + luaScripts[selectedConfig];
                            ExecuteLuaScript(selectedFile);
                        }
                    }
                    ImGui::NextColumn();
                    if (ImGui::Button("Unload Lua", ImVec2(-1, 40))) {
                        UnloadLuaScript();
                    }

                    ImGui::NextColumn();
                    if (ImGui::Button("Delete Selected", ImVec2(-1, 40))) {
                        if (!luaScripts.empty()) {
                            DeleteLuaScript(luaScripts[selectedConfig]);
                        }
                    }
                    ImGui::NextColumn();
                    if (ImGui::Button("Open Folder", ImVec2(-1, 40))) {
                        std::string luaFolder = GetLuaScriptsPath();
                        ShellExecuteA(NULL, "open", luaFolder.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                    }

                    ImGui::Columns(1);
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("Save New Script:");

                    static char newScriptName[64] = "";
                    ImGui::PushItemWidth(200);
                    ImGui::InputText("##NewScript", newScriptName, IM_ARRAYSIZE(newScriptName));
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    if (ImGui::Button("Save As", ImVec2(100, 40))) {
                        if (strlen(newScriptName) > 0) {
                            CreateLuaScript(newScriptName);
                            RefreshLuaScriptsList();
                            memset(newScriptName, 0, sizeof(newScriptName));
                        }
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Edit Script", ImVec2(-1, 40))) {
                        if (!luaScripts.empty()) {
                            EditLuaScript(luaScripts[selectedConfig]);
                        }
                    }
                    ImGui::EndTabItem();
                    firstOpen = !ImGui::IsItemActive();
                }
                if (ImGui::BeginTabItem("Gui Changer")) {
                    // Contenido de la pestaña Gui Changer
                    ImGui::ColorEdit4("Background", (float*)&bgColor);
                    ImGui::ColorEdit4("Button Color", (float*)&buttonColor);
                    ImGui::ColorEdit4("Text", (float*)&textColor);
                    ImGui::ColorEdit4("Watermark Text", (float*)&watermarkTextColor);
                    ImGui::ColorEdit4("Watermark Background", (float*)&watermarkBackGroundColor);
                    ImGui::ColorEdit4("Checkbox Active", (float*)&checkboxActive);
                    ImGui::ColorEdit4("Frame Background", (float*)&Framebc);
                    ImGui::ColorEdit4("Frame Hovered", (float*)&FrameHover);
                    ImGui::SliderFloat("Watermark Transparency", &watermarkTransparency, 0.0f, 1.0f, "Alpha: %.2f");

                    ImGui::Text("Build Date: %s", __DATE__);
                    ImGui::Text("Build Time: %s", __TIME__);

                    ImGui::Checkbox("Info", &info);

                    if (info) {
						ImGui::Text("Color Modes\n""organization and documentation");
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            const char* customTitle = "SunSet CS2";
            ImVec2 textSize = ImGui::CalcTextSize(customTitle);

            float x = windowPos.x + windowSize.x - textSize.x - 75.0f;
            float y = windowPos.y + 20.0f;

            ImGui::GetWindowDrawList()->AddText(
                ImVec2(x, y),
                IM_COL32(255, 180, 50, 255),
                customTitle
            );
        }


            RefreshConfigList();

            ImGui::PopStyleVar(3);
            ImGui::End();

            RenderWatermark(ping, fps);

            for (int playerIndex = 1; playerIndex < 32; ++playerIndex)
            {
                uintptr_t listenentry = mem.Read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);

                if (!listenentry)
                    continue;

                uintptr_t player = mem.Read<uintptr_t>(listenentry + 120 * (playerIndex & 0x1FF));

                if (!player)
                    continue;

                int playerTeam = mem.Read<int>(player + offset::m_iTeamNum);
                if (playerTeam == localTeam)
                    continue;

                uint32_t playerPawn = mem.Read<uint32_t>(player + offset::m_hPlayerPawn);

                uintptr_t listenentry2 = mem.Read<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FF) >> 9) + 16);

                if (!listenentry2)
                    continue;

                uintptr_t pCSPlayerPawn = mem.Read<uintptr_t>(listenentry2 + 120 * (playerPawn & 0x1FF));

                if (!pCSPlayerPawn)
                    continue;

                uintptr_t namePtr = mem.Read<uintptr_t>(player + offset::m_sSanitizedPlayerName);

                char playerName[128] = { 0 };

                if (namePtr) {
                    mem.ReadArray(namePtr, playerName, sizeof(playerName));

                    const char* weaponName = GetWeaponName(pCSPlayerPawn, mem);

                    int health = mem.Read<int>(pCSPlayerPawn + offset::m_iHealth);
                    int money = mem.Read<int>(pCSPlayerPawn + offset::m_iAccount);

                    if (health <= 0 || health > 100)
                        continue;

                    if (pCSPlayerPawn == localPlayer)
                        continue;

                    uint64_t gamescene = mem.Read<uint64_t>(pCSPlayerPawn + offset::m_pGameSceneNode);
                    uint64_t bonearray = mem.Read<uint64_t>(gamescene + offset::m_modelState + 0x80);

                    Vector3 origin = mem.Read<Vector3>(pCSPlayerPawn + offset::m_vOldOrigin);
                    Vector3 head = mem.Read<Vector3>(bonearray + bones::head * 32);

                    Vector3 screenPos = origin.WTS(view_matrix);
                    Vector3 screenHead = head.WTS(view_matrix);
                    float headheight = (screenPos.y - screenHead.y) / 8;

                    float height = screenPos.y - screenHead.y;
                    float width = height / 2.4f;

                    const int maxHealth = 100;

                    float healthBarHeight = (float)health / maxHealth * height;

                    if (espEnabled) {

                        if (boxEnabled) {
							RGBs boxColorToUse;
                            switch (colorModeBox) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedBox);
                                boxColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedBox, ImVec4(boxColor.r, boxColor.g, boxColor.b, 1.0f));
                                boxColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                boxColorToUse = boxColor;
                                break;
                            }
                            Render::DrawRect(screenHead.x - width / 2, screenHead.y, width, height, boxColorToUse, 1.5);
                        }

                        if (skeletonEnabled) {
                            RGBs skeletonColorToUse;
                            switch (colorModeSkeleton) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedSkeleton);
                                skeletonColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedSkeleton, ImVec4(skeletonColor.r, skeletonColor.g, skeletonColor.b, 1.0f));
                                skeletonColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                skeletonColorToUse = skeletonColor;
                                break;
                            }
                            Render::Circle(screenHead.x, screenHead.y, headheight - 3, skeletonColorToUse);
                        }

                        if (NameEsp) {
                            float nameX = screenHead.x - width / 2;
                            float nameY = screenHead.y - 15;
                            RGBs nameColorToUse;
                            switch (colorModeNameEsp) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedNameEsp);
                                nameColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedNameEsp, ImVec4(NameEspColor.r, NameEspColor.g, NameEspColor.b, 1.0f));
                                nameColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                nameColorToUse = NameEspColor;
                                break;
                            }
                            Render::DrawText(nameX, nameY, playerName, nameColorToUse);
                        }

                        if (WeaponEsp) {
                            float weaponX = screenHead.x - width / 2;
                            float weaponY = screenHead.y + height + 5;
							RGBs weaponColorToUse;
                            switch (colorModeWeaponEsp) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedWeaponEsp);
                                weaponColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedWeaponEsp, ImVec4(WeaponEspColor.r, WeaponEspColor.g, WeaponEspColor.b, 1.0f));
                                weaponColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                weaponColorToUse = WeaponEspColor;
                                break;
                            }
                            Render::DrawText(weaponX, weaponY, weaponName, weaponColorToUse);
                        }

                        if (HealTextEsp) {
                            char healthText[16];
                            snprintf(healthText, sizeof(healthText), "%d", health);

                            float textX = screenHead.x + width / 2 + 10;
                            float textY = screenHead.y + height / 2 - 7;
							RGBs healTextColorToUse;
                            switch (colorModeHealTextEsp) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedHealTextEsp);
                                healTextColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedHealTextEsp, ImVec4(HealTextEspColor.r, HealTextEspColor.g, HealTextEspColor.b, 1.0f));
                                healTextColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                healTextColorToUse = HealTextEspColor;
                                break;
                            }
                            Render::DrawText(textX, textY, healthText, healTextColorToUse);
                        }

                        if (healthEnabled) {
                            float healthBarXStart = screenHead.x + width / 2 + 5;
                            float healthBarYStart = screenHead.y + height;
                            float healthBarYEnd = screenHead.y + height - healthBarHeight;
							RGBs healthColorToUse;
                            switch (colorModeHealth) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedHealth);
                                healthColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedHealth, ImVec4(healthColor.r, healthColor.g, healthColor.b, 1.0f));
                                healthColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                healthColorToUse = healthColor;
                                break;
                            }

                            Render::Line(healthBarXStart, screenHead.y + height, healthBarXStart, screenHead.y, RGBs{ 102 / 255.0f, 102 / 255.0f, 102 / 255.0f }, 2.0f);
                            Render::Line(healthBarXStart, healthBarYStart, healthBarXStart, healthBarYEnd, healthColorToUse, 2.0f);
                        }

                        if (skeletonEnabled) {
                            RGBs skeletonColorToUse;
                            switch (colorModeSkeleton) {
                            case Rainbow: {
                                ImVec4 c = GetRainbowColor(SpeedSkeleton);
                                skeletonColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Pulse: {
                                ImVec4 c = GetPulseColor(SpeedSkeleton, ImVec4(skeletonColor.r, skeletonColor.g, skeletonColor.b, 1.0f));
                                skeletonColorToUse = { c.x, c.y, c.z };
                                break;
                            }
                            case Static:
                            default:
                                skeletonColorToUse = skeletonColor;
                                break;
                            }
                            for (const auto& connection : boneConnections) {
                                int bone1 = connection.bone1;
                                int bone2 = connection.bone2;

                                Vector3 VectorBone1 = mem.Read<Vector3>(bonearray + bone1 * 32);
                                Vector3 VectorBone2 = mem.Read<Vector3>(bonearray + bone2 * 32);

                                if (VectorBone1.IsValid() && VectorBone2.IsValid()) {
                                    Vector3 b1 = VectorBone1.WTS(view_matrix);
                                    Vector3 b2 = VectorBone2.WTS(view_matrix);
                                    Render::Line(b1.x, b1.y, b2.x, b2.y, skeletonColorToUse, skeletonLineThickness);
                                }
                            }
                        }
                    }
                }
            }


            ImGui::Render();
            float color[4]{ 0, 0, 0, 0 };
            device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
            device_context->ClearRenderTargetView(render_target_view, color);

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            swap_chain->Present(0U, 0U);
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();

        ImGui::DestroyContext();

        if (swap_chain) {
            swap_chain->Release();
        }

        if (device_context) {
            device_context->Release();
        }

        if (device) {
            device->Release();
        }

        if (render_target_view) {
            render_target_view->Release();
        }

        DestroyWindow(overlay);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);

        return 0;
    }

    const char* GetWeaponName(uint64_t csPlayerPawn, Memory& mem) {
        static char weaponNameBuffer[64]; // Tamaño suficiente para el nombre
        weaponNameBuffer[0] = '\0';

        uintptr_t activeWeapon = mem.Read<uintptr_t>(csPlayerPawn + offset::m_pClippingWeapon);
        if (!activeWeapon) {
            strcpy_s(weaponNameBuffer, "Invalid Weapon!");
            return weaponNameBuffer;
        }

        uint16_t itemDefinitionIndex = mem.Read<uint16_t>(activeWeapon + offset::m_AttributeManager + offset::m_Item + offset::m_iItemDefinitionIndex);

        for (const auto& weapon : IndexToWeapon) {
            if (weapon.first == itemDefinitionIndex) {
                strncpy_s(weaponNameBuffer, weapon.second.c_str(), sizeof(weaponNameBuffer) - 1);
                weaponNameBuffer[sizeof(weaponNameBuffer) - 1] = '\0';
                return weaponNameBuffer;
            }
        }
        strcpy_s(weaponNameBuffer, "Unknown Weapon");
        return weaponNameBuffer;
    }
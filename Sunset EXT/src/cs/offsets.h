#ifndef OFFSETS_H
#define OFFSETS_H

#include <cstddef>
#include <string>
#include <unordered_map>

namespace offset {
    inline std::ptrdiff_t dwViewMatrix;
    inline std::ptrdiff_t dwLocalPlayerPawn;
    inline std::ptrdiff_t dwEntityList;
    inline std::ptrdiff_t dwLocalPlayerController;
    inline std::ptrdiff_t dwViewAngles;

    inline std::ptrdiff_t m_flFlashBangTime;
    inline std::ptrdiff_t m_iIDEntIndex;
    inline std::ptrdiff_t m_fFlags;
    inline std::ptrdiff_t v_angle;
    inline std::ptrdiff_t m_iHealth;
    inline std::ptrdiff_t m_iTeamNum;
    inline std::ptrdiff_t m_vOldOrigin;
    inline std::ptrdiff_t m_hPlayerPawn;

    inline std::ptrdiff_t m_pGameSceneNode;
    inline std::ptrdiff_t m_modelState;
    inline std::ptrdiff_t m_entitySpottedState;
    inline std::ptrdiff_t m_pPawnSubclass;

    inline std::ptrdiff_t m_sSanitizedPlayerName;
    inline std::ptrdiff_t m_pClippingWeapon;
    inline std::ptrdiff_t m_AttributeManager;
    inline std::ptrdiff_t m_Item;
    inline std::ptrdiff_t m_iItemDefinitionIndex;
    inline std::ptrdiff_t m_iAccount;
    inline std::ptrdiff_t m_hPawn;
    inline std::ptrdiff_t m_bIsLocalPlayerController;
    inline std::ptrdiff_t m_iPing;

    // función para inicializar desde un JSON cargado
    void loadFromJson(const std::unordered_map<std::string, std::string>& data);
}

#endif
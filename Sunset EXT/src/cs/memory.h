#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>

#include <cstdint>
#include <string_view>
#include <vector>

class Memory
{
private:
    std::uintptr_t processId = 0;
    void* processHandle = nullptr;

public:
    // Constructor que encuentra el id del proceso
    // y abre un handle
    Memory(const std::string_view processName) noexcept
    {
        ::PROCESSENTRY32 entry = { };
        entry.dwSize = sizeof(::PROCESSENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        while (::Process32Next(snapShot, &entry))
        {
            if (!processName.compare(entry.szExeFile))
            {
                processId = entry.th32ProcessID;
                processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                break;
            }
        }

        // Liberar handle
        if (snapShot)
            ::CloseHandle(snapShot);
    }

    // Destructor que libera el handle abierto
    ~Memory()
    {
        if (processHandle)
            ::CloseHandle(processHandle);
    }

    // Devuelve la direcci?n base de un m?dulo por nombre
    const std::uintptr_t GetModuleAddress(const std::string_view moduleName) const noexcept
    {
        ::MODULEENTRY32 entry = { };
        entry.dwSize = sizeof(::MODULEENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);

        std::uintptr_t result = 0;

        while (::Module32Next(snapShot, &entry))
        {
            if (!moduleName.compare(entry.szModule))
            {
                result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                break;
            }
        }

        if (snapShot)
            ::CloseHandle(snapShot);

        return result;
    }

    // Leer memoria del proceso
    template <typename T>
    constexpr const T Read(const std::uintptr_t& address) const noexcept
    {
        T value = { };
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
        return value;
    }

    // Escribir memoria del proceso
    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
    {
        ::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
    }

    // Leer una cadena de la memoria del proceso
    void ReadString(const std::uintptr_t& address, char* buffer, std::size_t size) const noexcept
    {
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), buffer, size, NULL);
    }

    // Leer un array de la memoria del proceso
    template <typename T>
    void ReadArray(const std::uintptr_t& address, T* buffer, std::size_t size) const noexcept
    {
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), buffer, size * sizeof(T), NULL);
    }

    // Leer datos en bruto de la memoria del proceso
    bool ReadRaw(const std::uintptr_t& address, void* buffer, std::size_t size) const noexcept
    {
        return ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), buffer, size, NULL) != 0;
    }
};

// Declarar las variables globales
extern Memory mem;
extern std::uintptr_t client;
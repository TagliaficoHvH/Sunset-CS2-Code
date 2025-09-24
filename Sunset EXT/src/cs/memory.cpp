// memory.cpp
#include "memory.h"

// Definir la instancia global de Memory
Memory mem("cs2.exe"); // Reemplaza "cs2.exe" con el nombre real del proceso
std::uintptr_t client = mem.GetModuleAddress("client.dll"); // Inicializar la variable global del cliente
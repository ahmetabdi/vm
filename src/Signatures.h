#pragma once

#include <vector>

struct Signature
{
    const char* processName;
    const char* moduleName;
    const char* pattern;
    uintptr_t* result;

    template<typename T>
    Signature(const char *procName, const char* modName, const char* p, T& ref )
    : result((uintptr_t*)&ref), moduleName(modName), pattern(p), processName(procName) {}
};

inline std::vector<Signature> signatures = {
    // Example - Signature(entList, "[48 8D 15 *?? ?? ?? ??] 48 C1 E1 05 C1", MODNAME),
    // Signature(entList, "[48 8D 15 *?? ?? ?? ??] 48 C1 E1 05 C1", MODNAME),
};
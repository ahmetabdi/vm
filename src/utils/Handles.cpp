#include "hlapi/hlapi.h"
#include "m0dular/utils/handles.h"

/*
ModuleInfo Handles::GetModuleInfo(WinProcess *process, const char *module)
{
    WinDll *moduleInfo = process->GetModuleInfo(module);

    if (!moduleInfo)
        return { nullptr, 0, 0 };

    return { (void *)moduleInfo, moduleInfo->info.baseAddress, moduleInfo->info.sizeOfModule };
}
*/
#include "../Interface.h"
#include "../Common.h"
#include "../SDK/LE1SDK/SdkHeaders.h"


#define MYHOOK "LE1AutoloadIniLogger_"

SPI_PLUGINSIDE_SUPPORT(L"LE1AutoloadIniLogger", L"0.1.0", L"---", SPI_GAME_LE1, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// ProcessIni hook
// ======================================================================

typedef void (*tProcessIni)(TArray<FString>* OutFiles, wchar_t** IniPath, void* Something);
tProcessIni ProcessIni = nullptr;
tProcessIni ProcessIni_orig = nullptr;
void ProcessIni_hook(TArray<FString>* OutFiles, wchar_t** IniPath, void* Something)
{
    writeln(L"ProcessIni - IniPath is %s", *IniPath);
    ProcessIni_orig(OutFiles, IniPath, Something);
    
    for (int i = 0; OutFiles != nullptr && i < OutFiles->Count; i++)
    {
        writeln(L"ProcessIni - OutFiles[%d] is %s", i, OutFiles->Data[i].Data);
    }
}

// ======================================================================


SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();

    // Don't need to initialize SDK here
    // because this plugin only uses it for TArray and FString.

    if (auto rc = InterfacePtr->FindPattern((void**)&ProcessIni, "40 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 A0 EC FF FF B8 60 14 00 00"); 
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to find ProcessIni pattern: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    if (auto rc = InterfacePtr->InstallHook(MYHOOK "ProcessIni", ProcessIni, ProcessIni_hook, (void**)&ProcessIni_orig); 
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to hook ProcessIni: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    return true;
}

SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

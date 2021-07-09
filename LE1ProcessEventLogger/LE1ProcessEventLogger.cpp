#include "../Interface.h"
#include "../Common.h"
#include "../SDK/LE1SDK/SdkHeaders.h"


#define MYHOOK "LE1PELogger_"

SPI_PLUGINSIDE_SUPPORT(L"LE1ProcessEventLogger", L"0.1.0", L"d00telemental", SPI_GAME_LE1, SPI_VERSION_ANY);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// ProcessEvent hook
// ======================================================================

typedef void (*tProcessEvent)(UObject* Context, UObject* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;

tProcessEvent ProcessEvent_orig = nullptr;
void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
    writeln(L"PEHook - %S", Function->GetFullName());
    ProcessEvent_orig(Context, Function, Parms, Result);
}

// ======================================================================


SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();

    auto _ = SDKInitializer::Instance();
    writeln(L"Attach - names at 0x%p, objects at 0x%p",
        SDKInitializer::Instance()->GetBioNamePools(),
        SDKInitializer::Instance()->GetObjects());

    if (auto rc = InterfacePtr->FindPattern((void**)&ProcessEvent, "40 55 41 56 41 57 48 81 EC 90 00 00 00 48 8D 6C 24 20"); 
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to find ProcessEvent pattern: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    if (auto rc = InterfacePtr->InstallHook(MYHOOK "ProcessEvent", ProcessEvent, ProcessEvent_hook, (void**)&ProcessEvent_orig); 
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to hook ProcessEvent: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    return true;
}

SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

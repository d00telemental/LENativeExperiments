#include <vector>

#include "../Interface.h"
#include "../Common.h"

#include "PrivateUtils.h"
#include "../SDK/LE1SDK/SdkHeaders.h"


#define MYHOOK "LE1AutoloadIniLogger_"

SPI_PLUGINSIDE_SUPPORT(L"LE1AutoloadIniLogger", L"0.1.0", L"---", SPI_GAME_LE1, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// ProcessIni hook
// ======================================================================

bool GOriginalCalled = false;
std::vector<wchar_t*> GExtraAutoloadPaths{};

// NI: The first param is actually a class pointer.
typedef void (*tProcessIni)(TArray<FString>* OutFiles, FString* IniPath, void* Something);
tProcessIni ProcessIni = nullptr;
tProcessIni ProcessIni_orig = nullptr;
void ProcessIni_hook(TArray<FString>* OutFiles, FString* IniPath, void* Something)
{
    writeln(L"ProcessIni - IniPath is %s", IniPath->Data);
    ProcessIni_orig(OutFiles, IniPath, Something);
    
    if (!GOriginalCalled)
    {
        GOriginalCalled = true;

        for (auto autoloadPath : GExtraAutoloadPaths)
        {
            ProcessIni(OutFiles, &FString{ autoloadPath }, nullptr);
        }

        for (int i = 0; OutFiles != nullptr && i < OutFiles->Count; i++)
        {
            writeln(L"ProcessIni - OutFiles[%d] is %s", i, OutFiles->Data[i].Data);
        }
    }
}

// ======================================================================


// ProcessEvent hook
// ======================================================================

typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;

tProcessEvent ProcessEvent_orig = nullptr;
void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
    if (!strcmp(Function->GetFullName(), "Function SFXGame.BioHUD.PostRender"))
    {
        auto hudCanvas = ((ABioHUD*)Context)->Canvas;

        hudCanvas->SetPos(200.f, 200.f);
        hudCanvas->SetDrawColor(0xFF, 0x00, 0x00, 0xFF);
        hudCanvas->DrawTextW(FString{ L"THIS IS A WIP VERSION, USES A HARDCODED PATH!!!" }, 0, 1.25f, 1.25f, nullptr);
    }

    ProcessEvent_orig(Context, Function, Parms, Result);
}

// ======================================================================


SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();

    // Don't need to initialize SDK here
    // because this plugin only uses it for TArray and FString.


    // Find and hook some things.

    INIT_FIND_PATTERN(ProcessIni, "40 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 A0 EC FF FF B8 60 14 00 00");
    INIT_HOOK_PATTERN(ProcessIni);

    INIT_FIND_PATTERN(ProcessEvent, "40 55 41 56 41 57 48 81 EC 90 00 00 00 48 8D 6C 24 20");
    INIT_HOOK_PATTERN(ProcessEvent);


    // Get a list of DLC Autoloads.
    GExtraAutoloadPaths.push_back(L"D:\\Games Origin\\Mass Effect Legendary Edition\\Game\\ME1\\BioGame\\DLC\\DLC_Testi\\AutoLoad.ini");


    return true;
}

SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

#include "../Interface.h"
#include "../Common.h"
#include "PrivateUtils.h"
#include "PrivateClasses.h"
#include "Logging.h"
#include "../SDK/LE1SDK/SdkHeaders.h"


#define MYHOOK "LE1AutoloadIniLogger_"

SPI_PLUGINSIDE_SUPPORT(L"LE1AutoloadIniLogger", L"---", L"0.2.0", SPI_GAME_LE1, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// ProcessIni hook
// Logs every invocation, call itself for each found Autoload.ini.
// ======================================================================

bool GOriginalCalled = false;
ExtraContent* GExtraContent = nullptr;
std::vector<std::wstring> GExtraAutoloadPaths{};

// NI: The first param is actually a class pointer.
typedef void (*tProcessIni)(ExtraContent* ExtraContent, FString* IniPath, FString* BasePath);
tProcessIni ProcessIni = nullptr;
tProcessIni ProcessIni_orig = nullptr;
void ProcessIni_hook(ExtraContent* ExtraContent, FString* IniPath, FString* BasePath)
{
    writeln(L"ProcessIni - ExtraContent = %p, IniPath is %s", ExtraContent, IniPath->Data);
    ProcessIni_orig(ExtraContent, IniPath, BasePath);
    
    if (!GOriginalCalled)
    {
        GOriginalCalled = true;
        for (const auto& autoloadPath : GExtraAutoloadPaths)
        {
            ProcessIni(ExtraContent, &FString{ const_cast<wchar_t*>(autoloadPath.c_str()) }, nullptr);
        }
        GExtraContent = ExtraContent;
    }
}

// ======================================================================


// ProcessEvent hook
// Renders autoload profiler, allows toggling it.
// ======================================================================

#ifndef NDEBUG

typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;

tProcessEvent ProcessEvent_orig = nullptr;
void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
    static ExtraContentHUD* hud = nullptr;


    // Render autoload profiler HUD.

    if (!strcmp(Function->GetFullName(), "Function SFXGame.BioHUD.PostRender"))
    {
        if (!hud)
        {
            hud = new ExtraContentHUD{ !GIsRelease };
        }
        hud->Update(((ABioHUD*)Context)->Canvas, GExtraContent);
        hud->Draw();
    }


    // Allow toggling the autoload profiler HUD on `profile autoload` command.

    if (!strcmp(Function->GetFullName(), "Function Console.Typing.InputChar") && Parms && hud)
    {
        auto sfxConsole = reinterpret_cast<UConsole*>(Context);
        auto inputParms = reinterpret_cast<UConsole_execInputChar_Parms*>(Parms);
        if (inputParms->Unicode.Count >= 1 && inputParms->Unicode.Data[0] == L'\r'
            && sfxConsole->TypedStr.Data
            && !wcsncmp(sfxConsole->TypedStr.Data, L"profile ", 8))
        {
            hud->SetVisible(!wcscmp(sfxConsole->TypedStr.Data, L"profile autoload"));
        }
    }

    ProcessEvent_orig(Context, Function, Parms, Result);
}

#endif

// ======================================================================


SPI_IMPLEMENT_ATTACH
{
        initLog();
        writeln(L"Attach - hello there!");

#ifndef NDEBUG
        // Initialize the SDK because we need object names.
        INIT_CHECK_SDK();

        // Hook ProcessEvent for debugging.
        INIT_FIND_PATTERN(ProcessEvent, "40 55 41 56 41 57 48 81 EC 90 00 00 00 48 8D 6C 24 20");
        INIT_HOOK_PATTERN(ProcessEvent);
#endif

    // Find and hook the ProcessIni.

    INIT_FIND_PATTERN(ProcessIni, "40 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 A0 EC FF FF B8 60 14 00 00");
    INIT_HOOK_PATTERN(ProcessIni);

    // Get a list of DLC Autoloads.

    for (const auto& autoload : GetAllDLCAutoloads(GetDLCsRoot()))
    {
        writeln(L"Attach - found a DLC autoload: %s", autoload.c_str());
        GExtraAutoloadPaths.push_back(autoload.c_str());
    }

    return true;
}

SPI_IMPLEMENT_DETACH
{
    closeLog();
    return true;
}

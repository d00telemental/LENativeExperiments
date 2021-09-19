#include "../Interface.h"
#include "../Common.h"


SPI_PLUGINSIDE_SUPPORT(L"LE1GPUCrusher", L"0.1.0", L"d00telemental", SPI_GAME_LE1, SPI_VERSION_ANY);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;

#define writeln(frmt, ...)   fwprintf_s(stdout, L"LE1GPUCrusher - " frmt "\n", __VA_ARGS__);


#define REFLECTIONUPDATEREQUIRED_PATTERN "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 83 EC 40 48 8B 01 48 8B F2 4C 8B F9 FF 50 48"
using tReflectionUpdateRequired = unsigned long (*)(void*, void*);
tReflectionUpdateRequired ReflectionUpdateRequired = nullptr;
tReflectionUpdateRequired ReflectionUpdateRequired_orig = nullptr;
unsigned long ReflectionUpdateRequired_hook(void* a, void* b)
{
    const auto value = ReflectionUpdateRequired_orig(a, b);

    // !!! DANGER !!!
    return TRUE;
    // !!! DANGER !!!

    writeln(L"ReflectionUpdateRequired_orig => %d", value);
    return value;
}


// --------------------------------------------------
SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();
    writeln(L"Attach - hello! DO NOT USE IF YOU AREN'T WILLING TO REPLACE YOUR GPU");

    if (auto rc = InterfacePtr->FindPattern((void**)&ReflectionUpdateRequired, REFLECTIONUPDATEREQUIRED_PATTERN);
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to find the pattern: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }
    writeln(L"Attach - found pattern at %p!", ReflectionUpdateRequired);

    if (auto rc = InterfacePtr->InstallHook("LE1GPUCrusherReflectionUpdateRequired", ReflectionUpdateRequired, ReflectionUpdateRequired_hook, (void**)&ReflectionUpdateRequired_orig);
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to hook ReflectionUpdateRequired: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    Sleep(15 * 1000);

    MessageBoxW(nullptr, L"DO NOT USE IF YOU AREN'T WILLING TO REPLACE YOUR GPU", L"DANGER", MB_OK | MB_ICONERROR);

    return true;
}

// --------------------------------------------------
SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

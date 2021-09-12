#include "../Interface.h"
#include "../Common.h"
#include <cstdio>
#include <Windows.h>
#include "../SDK/LE2SDK/SdkHeaders.h"

#define MYHOOK "LE2TestProject_"

SPI_PLUGINSIDE_SUPPORT(L"LE2TestProject", L"d00t", L"0.1.0", SPI_GAME_LE2, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// Hook macros.
// ========================================

#define _CONCAT_NAME(A, B)  A ## B
#define CONCAT_NAME(A, B)   _CONCAT_NAME(A, B)
#define _STR(X)             #X
#define STR(X)              _STR(X)
#define _ADD_QUOTES(S)      #S
#define ADD_QUOTES(S)       _ADD_QUOTES(S)
#define WCSTR(T)            L"" T

#define INIT_CHECK_SDK() \
    auto _ = SDKInitializer::Instance(); \
    if (!SDKInitializer::Instance()->GetBioNamePools()) \
    { \
        fwprintf_s(stdout, L"Attach - GetBioNamePools() returned NULL!\n"); \
        return false; \
    } \
    if (!SDKInitializer::Instance()->GetObjects()) \
    { \
        fwprintf_s(stdout, L"GetObjects() returned NULL!\n"); \
        return false; \
    }

#define INIT_FIND_PATTERN(VAR, PATTERN) \
    if (auto rc = InterfacePtr->FindPattern((void**)&VAR, PATTERN); rc != SPIReturn::Success) \
    { \
        fwprintf_s(stdout, L"Attach - failed to find " #VAR L" posthook pattern: %d / %s\n", rc, SPIReturnToString(rc)); \
        return false; \
    } \
    VAR = (decltype(VAR))((char*)VAR - 5); \
    fwprintf_s(stdout, L"Attach - found " #VAR L" posthook pattern: 0x%p\n", VAR);

#define INIT_HOOK_PATTERN(VAR) \
    if (auto rc = InterfacePtr->InstallHook(MYHOOK #VAR, VAR, CONCAT_NAME(VAR, _hook), (void**)& CONCAT_NAME(VAR, _orig)); rc != SPIReturn::Success) \
    { \
        fwprintf_s(stdout, L"Attach - failed to hook " #VAR L": %d / %s\n", rc, SPIReturnToString(rc)); \
        return false; \
    } \
    fwprintf_s(stdout, L"Attach - hooked " #VAR L": 0x%p -> 0x%p (saved at 0x%p)\n", VAR, CONCAT_NAME(VAR, _hook), CONCAT_NAME(VAR, _orig));


// Primitive ProcessEvent hook.
// ========================================

#define LE2TP_PROCESSEVENT   /*"40 55 41 56 41*/ "57 48 81 EC 90 00 00 00 48 8D 6C 24 20 48 C7 45 50 FE FF FF FF 48 89 9D 90 00 00 00 48 89 B5 98 00 00 00 48 89 BD A0 00 00 00 4C 89 A5 A8 00 00 00 48 8B"
typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;
tProcessEvent ProcessEvent_orig = nullptr;
void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
    if ( ! strcmp( Function->GetFullName(), "Function SFXGame.BioHUD.PostRender" ) )
    {
        auto bioHud = reinterpret_cast<ABioHUD*>(Context);
        if (bioHud && bioHud->IsA(ABioHUD::StaticClass()))
        {
            FFontRenderInfo ffri{};

            bioHud->Canvas->SetPos(100.f, 100.f);
            bioHud->Canvas->SetDrawColor(0xFF, 0x00, 0x00, 0xFF);
            bioHud->Canvas->DrawTextW(FString{ L"Hello there!" }, TRUE, 1.f, 1.f, &ffri);
        }
    }

    ProcessEvent_orig(Context, Function, Parms, Result);
}


// SPI impl.
// ========================================

SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();
    fwprintf_s(stdout, L"Hello there!\n");

    INIT_CHECK_SDK();
    INIT_FIND_PATTERN(ProcessEvent, LE2TP_PROCESSEVENT);
    INIT_HOOK_PATTERN(ProcessEvent);

    return true;
}

SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

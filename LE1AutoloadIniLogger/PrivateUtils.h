#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <Windows.h>
#include "../SDK/LE1SDK/SdkHeaders.h"

#pragma comment(lib, "Shlwapi.lib")
#include "Shlwapi.h"


#ifndef NDEBUG
static bool GIsRelease = false;
#else
static bool GIsRelease = true;
#endif


#define _CONCAT_NAME(A, B) A ## B
#define CONCAT_NAME(A, B) _CONCAT_NAME(A, B)


// SDK and SDK hooks initialization macros.
// MUST ONLY BE USED IN SPI ATTACH!
// ======================================================================

#define INIT_CHECK_SDK() \
    auto _ = SDKInitializer::Instance(); \
    if (!SDKInitializer::Instance()->GetBioNamePools()) \
    { \
        writeln(L"Attach - SDK initialization returns NULL BioName pools!"); \
        return false; \
    } \
    if (!SDKInitializer::Instance()->GetObjects()) \
    { \
        writeln(L"Attach - SDK initialization returns NULL GObjObjects array!"); \
        return false; \
    }

#define INIT_FIND_PATTERN(VAR, PATTERN) \
    if (auto rc = InterfacePtr->FindPattern((void**)&VAR, PATTERN); rc != SPIReturn::Success) \
    { \
        writeln(L"Attach - failed to find " #VAR L"pattern: %d / %s", rc, SPIReturnToString(rc)); \
        return false; \
    }

#define INIT_HOOK_PATTERN(VAR) \
    if (auto rc = InterfacePtr->InstallHook(MYHOOK #VAR, VAR, CONCAT_NAME(VAR, _hook), (void**)& CONCAT_NAME(VAR, _orig)); rc != SPIReturn::Success) \
    { \
        writeln(L"Attach - failed to hook " #VAR L": %d / %s", rc, SPIReturnToString(rc)); \
        return false; \
    }


// A non-default UObject searching function.
// ======================================================================

template <typename T>
T* FindObject() noexcept
{
    auto objects = UObject::GObjObjects();
    for (auto i = 0; i < objects->Count; i++)
    {
        auto object = (*objects)(i);
        if (object && object->IsA(T::StaticClass()) && strcmp(object->Name.GetName(), "Default_"))
        {
            return (T*)object;
        }
    }
    return nullptr;
}


// Common UObject accessory functions.
// ======================================================================

UEngine* GetEngine() noexcept
{
    return FindObject<UEngine>();
}
ABioWorldInfo* GetWorldInfo() noexcept
{
    auto engine = GetEngine();
    if (!engine) return nullptr;
    return (ABioWorldInfo*)engine->GetCurrentWorldInfo();
}
ABioPlayerController* GetPlayerController() noexcept
{
    auto worldInfo = GetWorldInfo();
    if (!worldInfo) return nullptr;
    return (ABioPlayerController*)worldInfo->LocalPlayerController;
}
ABioPawn* GetPlayerPawn() noexcept
{
    auto playerCtrl = GetPlayerController();
    if (!playerCtrl) return nullptr;
    return (ABioPawn*)playerCtrl->Pawn;
}
UBioPawnBehavior* GetPlayerBehavior() noexcept
{
    auto playerPawn = GetPlayerPawn();
    if (!playerPawn) return nullptr;
    return playerPawn->m_oBehavior;
}
ABioBaseSquad* GetPlayerSquad()
{
    auto playerBehavior = GetPlayerBehavior();
    if (!playerBehavior) return nullptr;
    return playerBehavior->GetSquad();
}
ABioSPGame* GetBioSPGame()
{
    auto worldInfo = GetWorldInfo();
    if (!worldInfo) return nullptr;
    return (ABioSPGame*)worldInfo->Game;
}


// Functions used to make a list of available DLCs.
// ======================================================================

std::wstring GetDLCsRoot()
{
    wchar_t modulePath[512];

    GetModuleFileNameW(nullptr, modulePath, 512);
    PathRemoveFileSpec(modulePath);
    PathRemoveFileSpec(modulePath);
    PathRemoveFileSpec(modulePath);

    std::wstring root;
    root.append(modulePath);
    root.append(L"\\BioGame\\DLC\\");

    return root;
}

std::vector<std::wstring> GetAllDLCAutoloads(std::wstring&& searchRoot)
{
    std::vector<std::wstring> autoloadPaths{};
    searchRoot.append(L"*");

    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFileW(searchRoot.c_str(), &fd);
    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && wcslen(fd.cFileName) > 4
            && fd.cFileName[0] == L'D' && fd.cFileName[1] == L'L' && fd.cFileName[2] == L'C' && fd.cFileName[3] == L'_')
        {
            wchar_t autoloadPath[512];
            swprintf_s(autoloadPath, 512, L"..\\..\\BioGame\\DLC\\%s\\AutoLoad.ini", fd.cFileName);
            autoloadPaths.emplace_back(autoloadPath);
        }
    } while (FindNextFile(handle, &fd) != 0);

    return autoloadPaths;
}


// Class / object mixins.
// ======================================================================

class NonCopyMovable
{
public:
    NonCopyMovable() = default;
    NonCopyMovable(const NonCopyMovable& other) = delete;
    NonCopyMovable(NonCopyMovable&& other) = delete;
    NonCopyMovable& operator=(const NonCopyMovable& other) = delete;
    NonCopyMovable& operator=(NonCopyMovable&& other) = delete;
};

class NonConstructible
{
public:
    NonConstructible() = delete;
    ~NonConstructible() = delete;
};

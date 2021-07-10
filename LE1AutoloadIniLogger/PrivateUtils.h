#pragma once
#include <Windows.h>
#include "../SDK/LE1SDK/SdkHeaders.h"


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


// Class / object mixins.
// ======================================================================

class NonCopyMovable
{
public:
    NonCopyMovable(const NonCopyMovable& other) = delete;
    NonCopyMovable(NonCopyMovable&& other) = delete;
    NonCopyMovable& operator=(const NonCopyMovable& other) = delete;
    NonCopyMovable& operator=(NonCopyMovable&& other) = delete;
};

class NonConstructible
{
    NonConstructible() = delete;
    ~NonConstructible() = delete;
};

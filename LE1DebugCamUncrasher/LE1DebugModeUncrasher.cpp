#include "../Interface.h"
#include "../Common.h"


SPI_PLUGINSIDE_SUPPORT(L"LE1DebugModeUncrasher", L"0.1.0", L"d00telemental", SPI_GAME_LE1, SPI_VERSION_ANY);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// This is the function which crashes after 'toggledebugcamera':
// ================================================================

// DWORD UGFxMovie::SomeMethod()
// {
//     int localPlayerIndex = this->LocalPlayerOwnerIndex;
//     if (localPlayerIndex < 0)
//     {
//         localPlayerIndex = 0;
//     }
//     APlayerController* localController = nullptr;
//     UGameEngine* engine = Cast<UGameEngine*>(GEngine);
//     if (localPlayerIndex < engine->GamePlayers.Count)
//     {
//         ULocalPlayer* localPlayer = engine->GamePlayers(localPlayerIndex);
//         if (localPlayer)
//         {
//             localController = localPlayer->Actor;
//         }
//     }
//     UPlayerInput* playerInput = localController->PlayerInput;  // <-- this is where the crash happens, playerInput is NULL
//     if (!playerInput->bUsingGamepad)
//     {
//         return 0;
//     }
//     return SomeControllerInputMethod();  // don't care to reverse engineer this
// }

// ================================================================
// The code below patches out the last 6 lines out,
// which means that controller doesn't work,
// but at least the game doesn't crash outright.


// --------------------------------------------------
SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();
    writeln(L"Attach - hello!");

    byte* crashingMethod;
    char crashingPattern[] = "48 83 EC 28 8B 81 88 01 00 00 85 C0 79 0C C7 81 88 01 00 00 00 00 00 00 33 C0";
    char replacementBytes[] = "\x90\x90\x90\x90\x90\x90\x90\xEB";
    uint64 replacementOffset = 0x4B;
    
    if (auto rc = InterfacePtr->FindPattern((void**)&crashingMethod, crashingPattern); 
        rc != SPIReturn::Success)
    {
        writeln(L"Attach - failed to find the pattern: %d / %s", rc, SPIReturnToString(rc));
        return false;
    }

    writeln(L"Attach - found pattern at %p, destination is %p !",
        crashingMethod, crashingMethod + replacementOffset);

    // Use RAII to automatically restore the memory protection on scope exit.
    struct RAIIMemoryPatchingContext
    {
        byte* Target;
        char* Replacement;
        size_t Length;
        int32 OldProtect;
        bool NeedsDestructor;

        RAIIMemoryPatchingContext(byte* target, char* replacement, size_t length)
            : Target{ target }, Replacement{ replacement }, Length{ length }, NeedsDestructor{ false }
        {
            if (!VirtualProtect(target, length, PAGE_EXECUTE_READWRITE, (DWORD*)&OldProtect))
            {
                writeln(L"Attach - failed to unprotect the memory, last error = %d", GetLastError());
                return;
            }

            memcpy(target, replacement, length);
            writeln(L"Attach - UGFxMovie::SomeMethod() patched!");
            NeedsDestructor = true;
        }

        ~RAIIMemoryPatchingContext()
        {
            if (NeedsDestructor)
            {
                VirtualProtect(Target, Length, OldProtect, (DWORD*)&OldProtect);
            }
        }
    } patchingContext{ crashingMethod + replacementOffset, replacementBytes, sizeof replacementBytes - 1 };

    return true;
}

// --------------------------------------------------
SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

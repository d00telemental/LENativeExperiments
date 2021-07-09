#pragma once

#define _CONCAT_NAME(A, B) A ## B
#define CONCAT_NAME(A, B) _CONCAT_NAME(A, B)


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

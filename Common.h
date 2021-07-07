#pragma once
#include <windows.h>
#include <TlHelp32.h>

#include <cstdio>
#include <string>


typedef unsigned char        byte;
typedef signed long          int32;
typedef unsigned long        uint32;
typedef signed long long     int64;
typedef unsigned long long   uint64;
typedef wchar_t wchar;


namespace Common
{
    void OpenConsole()
    {
        AllocConsole();

        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);

        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
        GetConsoleScreenBufferInfo(console, &lpConsoleScreenBufferInfo);
        SetConsoleScreenBufferSize(console, { lpConsoleScreenBufferInfo.dwSize.X, 30000 });
    }

    void CloseConsole()
    {
        FreeConsole();
    }

    // loosely based on https://stackoverflow.com/q/26572459
    byte* GetModuleBaseAddress(wchar* moduleName)
    {
        auto pid = GetCurrentProcessId();

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        byte* baseAddress = nullptr;

        if (INVALID_HANDLE_VALUE != snapshot)
        {
            MODULEENTRY32 moduleEntry = { 0 };
            moduleEntry.dwSize = sizeof(MODULEENTRY32);

            if (Module32First(snapshot, &moduleEntry))
            {
                do
                {
                    if (0 == wcscmp(moduleEntry.szModule, moduleName))
                    {
                        baseAddress = moduleEntry.modBaseAddr;
                        break;
                    }
                } while (Module32Next(snapshot, &moduleEntry));
            }
            CloseHandle(snapshot);
        }

        return baseAddress;
    }
}

#define writeln(msg,...) fwprintf_s(stdout, L"LENE::" msg "\n", __VA_ARGS__)

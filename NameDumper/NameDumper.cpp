#include "../Interface.h"
#include "../Common.h"


SPI_PLUGINSIDE_SUPPORT(L"NameDumper", L"0.1.0", L"d00telemental", SPI_GAME_LE1 | SPI_GAME_LE2 | SPI_GAME_LE3, SPI_VERSION_ANY);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


#define writeln(frmt, ...)   fwprintf_s(stdout, frmt "\n", __VA_ARGS__);


#pragma pack(4)
template <typename T>
struct TArray
{
    T* Data;
    int Count;
    int Max;
};
typedef TArray<wchar_t> FString;

/** Packed index DWORD as seen in FNameEntry. */
struct PackedIndex
{
    uint32 Offset : 20;  // The actual index, I guess???
    uint32 Length : 9;   // Length of the AnsiName or WideName in symbols \wo null-terminator.
    uint32 Bits : 3;     // Always 4 or 0. No idea wtf it really is, flags maybe?
};

#pragma pack(1)
/** Name as seen in some kind of name pool. */
struct FNameEntry
{
    PackedIndex Index;     // 0x00
    FNameEntry* HashNext;  // 0x04  Some pointer, often NULL.
    char AnsiName[1];      // 0x0C  This *potentially* can be a widechar.
};

#pragma pack(1)
/** Name reference as seen in individual UObjects. */
struct FName
{
    uint32 Offset : 29;  // Binary offset into an individual chunk.
    uint32 Chunk : 3;    // Index of the chunk, I've only seen 0 or 1.
    int32 Number;        // ?= InstanceIndex
};


// --------------------------------------------------
SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();
    writeln(L"Attach - hello!");

    SPIGameVersion hostGame;
    InterfacePtr->GetHostGame(&hostGame);
    writeln(L"Attach - host game is %d", static_cast<int>(hostGame));

    // Set game-dependent parameters.

    wchar* varModuleName = nullptr;
    uint64 varChunksOffset = 0;

    switch (hostGame)
    {
    case SPIGameVersion::LE1:
        varModuleName = L"MassEffect1.exe";
        varChunksOffset = 0x16A2090;
        break;
    case SPIGameVersion::LE2:
        varModuleName = L"MassEffect2.exe";
        varChunksOffset = 0x1668A10;
        break;
    case SPIGameVersion::LE3:
        varModuleName = L"MassEffect3.exe";
        varChunksOffset = 0x17B33D0;
        break;
    }

    auto moduleBase = Common::GetModuleBaseAddress(varModuleName);
    auto chunksBase = moduleBase + varChunksOffset;
    writeln(L"Attach - module base is %p, chunks base is %p", moduleBase, chunksBase);
    
    int poolCounter = 0;
    for (FNameEntry** namePool = reinterpret_cast<FNameEntry**>(chunksBase);
         *namePool != nullptr;
         namePool++)
    {
        int entryCounter = 0;
        for (FNameEntry* nameEntry = *namePool;
             nameEntry->Index.Length != 0;
             nameEntry = reinterpret_cast<FNameEntry*>(reinterpret_cast<byte*>(nameEntry) + sizeof FNameEntry + nameEntry->Index.Length))
        {
            writeln(L"NameDump[%d][%d] %S", poolCounter, entryCounter, nameEntry->AnsiName);
            entryCounter++;
        }
        poolCounter++;
    }

    return true;
}

// --------------------------------------------------
SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

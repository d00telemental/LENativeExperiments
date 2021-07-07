#include "../Interface.h"
#include "../Common.h"


SPI_PLUGINSIDE_SUPPORT(L"NameResearch", L"0.1.0", L"d00telemental", SPI_GAME_LE1, SPI_VERSION_ANY);

SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


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
    uint32 Chunk : 3;    // != FName::Chunk, always = 4. No idea wtf it really is.
};

#pragma pack(4)
/** Name as seen in some kind of name pool. */
struct FNameEntry
{
    PackedIndex Index;     // 0x00
    FNameEntry* HashNext;  // 0x04
    union                  // 0x0C
    {
        char AnsiName[1];
        wchar WideName[1];
    };
};

/** Some kind of pointer sequence (only ever seen 2) to ? name pools ?. */
struct FBioChunkedNameArray
{
    FNameEntry** Chunks;
} Names;

#pragma pack(4)
/** Name reference as seen in individual UObjects. */
struct FName
{
    uint32 Offset : 29;  // Binary offset (as in char* + Offset, not FNameEntry* + Offset) into individual "Chunk".
    uint32 Chunk : 3;    // Offset into FBioChunkedNameArray::Chunks, I've only seen 0 or 1.
    int32 Number;        // AKA InstanceIndex

    /**
     * Quickly hacked function which returns the internal AnsiName pointer.
     * The assumption is that there are no wide FNames in LE1.
     */
    __forceinline char* ToCharPtrQuick() const noexcept
    {
        auto chunk = Names.Chunks[Chunk];
        auto entry = (FNameEntry*)((byte*)chunk + Offset);
        return entry->AnsiName;
    }

    /**
     * Poor man's FName::ToString() which wraps the internal AnsiName pointer into an STL wide string.
     * I really want the original, but that'd require reverse-engineering the FNameEntry pool allocator.
     */
    std::wstring ToStlStringSlow() const
    {
        auto chunk = Names.Chunks[Chunk];
        auto nameEntry = (FNameEntry*)((byte*)chunk + Offset);

        wchar resultBuffer[1024];
        size_t resultBufferLength = ::MultiByteToWideChar(0, 0, nameEntry->AnsiName, nameEntry->Index.Length + 1, resultBuffer, 1024);
        if (resultBufferLength <= 0)
        {
            return std::wstring{ L"<FAILED TO CONVERT (" + std::to_wstring(GetLastError()) + L")>" };
        }

        return std::wstring{ resultBuffer, resultBufferLength };
    }
};


// --------------------------------------------------
SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();
    writeln(L"Attach - hello!");

    auto moduleBase = Common::GetModuleBaseAddress(L"MassEffect1.exe");
    writeln(L"Attach - module base is %p", moduleBase);

    Names.Chunks = reinterpret_cast<FNameEntry**>(0x16A2090 + moduleBase);

    for (size_t i = 0; i < 2; i++)
    {
        FNameEntry* nameEntry = Names.Chunks[i];
        int nameCounter = 0;
        do
        {
            if (nameEntry->Index.Length == 0)
            {
                break;
            }

            writeln(L"Attach - nameEntry[%llu][%d]: offset = %d, len = %d, ? = %d; ?? = %p; text = %S",
                i, nameCounter++,
                nameEntry->Index.Offset, nameEntry->Index.Length, nameEntry->Index.Chunk,
                nameEntry->HashNext, *&(nameEntry->AnsiName));

            nameEntry = (FNameEntry*)((byte*)nameEntry + 12 + nameEntry->Index.Length + 1);
        } while (true);
    }

    return true;
}

// --------------------------------------------------
SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

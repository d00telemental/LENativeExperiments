#pragma once
#include <Windows.h>
#include "../SDK/LE1SDK/SdkHeaders.h"
#include "PrivateUtils.h"


/// <summary>
/// Class used by BioWare instead of UE's built-in
/// DLC subsystem to load additional content.
/// </summary>
class ExtraContent final
{
public:
    struct TArray<struct FString> Package2DAs;
    struct TArray<struct FString> GlobalTlks;
    struct TArray<struct FString> PlotManagers;
    struct TArray<struct FString> EmptyArray1;
    struct TArray<struct FString> StateTransitionMaps;
    struct TArray<struct FString> ConsequenceMaps;

    struct TArray<struct FString> OutcomeMaps;
    struct TArray<struct FString> QuestMaps;
    struct TArray<struct FString> DataCodexMaps;
    struct TArray<struct FString> BioAutoConditionals;
    struct TArray<class UObject*> SomePackages;
};

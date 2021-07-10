#include <vector>
#include "../Interface.h"
#include "../Common.h"
#include "PrivateUtils.h"
#include "PrivateClasses.h"
#include "../SDK/LE1SDK/SdkHeaders.h"


#define MYHOOK "LE1AutoloadIniLogger_"

SPI_PLUGINSIDE_SUPPORT(L"LE1AutoloadIniLogger", L"0.1.0", L"---", SPI_GAME_LE1, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_POSTLOAD;
SPI_PLUGINSIDE_ASYNCATTACH;


// ProcessIni hook
// ======================================================================

bool GOriginalCalled = false;
ExtraContent* GExtraContent = nullptr;
std::vector<wchar_t*> GExtraAutoloadPaths{};

// NI: The first param is actually a class pointer.
typedef void (*tProcessIni)(ExtraContent* ExtraContent, FString* IniPath, FString* BasePath);
tProcessIni ProcessIni = nullptr;
tProcessIni ProcessIni_orig = nullptr;
void ProcessIni_hook(ExtraContent* ExtraContent, FString* IniPath, FString* BasePath)
{
    writeln(L"ProcessIni - ExtraContent = %p , IniPath is %s", ExtraContent, IniPath->Data);
    ProcessIni_orig(ExtraContent, IniPath, BasePath);
    
    if (!GOriginalCalled)
    {
        GOriginalCalled = true;

        for (auto autoloadPath : GExtraAutoloadPaths)
        {
            ProcessIni(ExtraContent, &FString{ autoloadPath }, nullptr);
        }

        GExtraContent = ExtraContent;

        //for (int i = 0; ExtraContent != nullptr && i < ExtraContent->Package2DAs.Count; i++)
        //{
        //    writeln(L"ProcessIni - ExtraContent->Package2DAs[%d] is %s", i, ExtraContent->Package2DAs(i).Data);
        //}
    }
}

// ======================================================================


// ProcessEvent hook
// ======================================================================

typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;

tProcessEvent ProcessEvent_orig = nullptr;
void ProcessEvent_hook(UObject* Context, UFunction* Function, void* Parms, void* Result)
{
    if (!strcmp(Function->GetFullName(), "Function SFXGame.BioHUD.PostRender"))
    {
        auto hudCanvas = ((ABioHUD*)Context)->Canvas;

        hudCanvas->SetPos(40.f, 10.f);
        hudCanvas->SetDrawColor(0xFF, 0x00, 0x00, 0xFF);
        hudCanvas->DrawTextW(FString{ L"THIS IS A WIP VERSION, USES A HARDCODED PATH!!!" }, 0, 1.f, 1.f, nullptr);

        if (GExtraContent)
        {
            FVector2D startPos{ 40.f, 40.f };
            FVector2D canvasSize{ (float)hudCanvas->SizeX, (float)hudCanvas->SizeY };

            const float columns = 5;
            const float columnSize = (canvasSize.X - startPos.X * 2 - 0.f) / columns - 10;
            
            const float rows = 2;
            const float rowSize = (canvasSize.Y - startPos.Y * 2 - 50.f) / rows;

            int row = 0, column = 0;

            hudCanvas->SetPos(0.f, 0.f);
            hudCanvas->SetDrawColor(0, 0, 0, 0x6F);
            hudCanvas->DrawRect(canvasSize.X, canvasSize.Y, hudCanvas->DefaultTexture);

            hudCanvas->SetPos(startPos.X, startPos.Y);
            hudCanvas->SetDrawColor(0x90, 0x80, 0x70, 0xFF);
            hudCanvas->DrawTextW(FString{ L"GExtraContent" }, 1, 1.f, 1.f, nullptr);

            hudCanvas->Draw2DLine(
                startPos.X, startPos.Y + 30.f,
                (float)hudCanvas->SizeX - startPos.X, startPos.Y + 30.f,
                FColor{ 0x70, 0x80, 0x90, 0xFF });

            hudCanvas->SetDrawColor(0x70, 0x80, 0x90, 0xFF);

            // Top row
            {
                column = 0;

                // 2DAs
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"2DAs:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->Package2DAs.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->Package2DAs(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // GlobalTlks
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"GlobalTlks:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->GlobalTlks.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->GlobalTlks(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // PlotManagers
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"PlotManagers:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->PlotManagers.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->PlotManagers(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // StateTransitionMaps
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"StateTransitionMaps:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->StateTransitionMaps.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->StateTransitionMaps(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // ConsequenceMaps
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"ConsequenceMaps:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->ConsequenceMaps.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->ConsequenceMaps(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                row++;
            }

            // Empty array

            // Bottom row
            {
                column = 0;

                // OutcomeMaps
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"OutcomeMaps:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->OutcomeMaps.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->OutcomeMaps(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // QuestMaps
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"QuestMaps:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->QuestMaps.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->QuestMaps(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // DataCodexMaps
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"DataCodexMaps:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->DataCodexMaps.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->DataCodexMaps(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // BioAutoConditionals
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"BioAutoConditionals:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->BioAutoConditionals.Num(); i++)
                    {
                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(GExtraContent->BioAutoConditionals(i), 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                // SomePackages
                {
                    hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 50.f + (rowSize * row));
                    hudCanvas->DrawTextW(FString{ L"SomePackages:" }, 1, 0.8f, 0.8f, nullptr);
                    for (auto i = 0; i < GExtraContent->SomePackages.Num(); i++)
                    {
                        auto object = GExtraContent->SomePackages(i);
                        wchar_t objectName[512];
                        swprintf_s(objectName, 512, L"%S %S", (object->Class ? object->Class->Name.GetName() : "WTF"), object->Name.GetName());

                        hudCanvas->SetPos(startPos.X + columnSize * column, startPos.Y + 70.f + (rowSize * row) + (i * 10.f));
                        hudCanvas->DrawTextW(objectName, 1, 0.8f, 0.8f, nullptr);
                    }
                    column++;
                }

                row++;
            }
        }
    }

    ProcessEvent_orig(Context, Function, Parms, Result);
}

// ======================================================================


SPI_IMPLEMENT_ATTACH
{
    Common::OpenConsole();

    // Initialize the SDK because we need object names.
    
    INIT_CHECK_SDK();


    // Find and hook some things.

    INIT_FIND_PATTERN(ProcessIni, "40 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 A0 EC FF FF B8 60 14 00 00");
    INIT_HOOK_PATTERN(ProcessIni);

    INIT_FIND_PATTERN(ProcessEvent, "40 55 41 56 41 57 48 81 EC 90 00 00 00 48 8D 6C 24 20");
    INIT_HOOK_PATTERN(ProcessEvent);


    // Get a list of DLC Autoloads.
    GExtraAutoloadPaths.push_back(L"..\\..\\BioGame\\DLC\\DLC_Testi\\AutoLoad.ini");


    return true;
}

SPI_IMPLEMENT_DETACH
{
    Common::CloseConsole();
    return true;
}

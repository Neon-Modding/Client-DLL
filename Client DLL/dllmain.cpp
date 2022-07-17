#include <Windows.h>
#include <MinHook/MinHook.h>

#include "detours.h"
#include "gui.h"
#include "helper.h"
#include "xorstr.hpp"

DWORD WINAPI Input(LPVOID)
{
    while (1)
    {
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            bLogEverything = true;
        }

        else if (GetAsyncKeyState(VK_F2) & 1)
        {
            bLogEverything = false;
        }

        else if (GetAsyncKeyState(VK_F3) & 1)
        {
            WriteToLog(_("\n\n\nNew\n"), _("OnRep_log"));
            bLogRep = true;
        }

        else if (GetAsyncKeyState(VK_F4) & 1)
        {
            bLogRep = false;
        }

        Sleep(100);
    }
}

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();

    FILE* file;
    freopen_s(&file, _("CONOUT$"), _("w"), stdout); // redirect the output to our new console.

    std::cout << _("Client DLL v1.1 by Milxnor.\n");

    std::string Binds = _(R"(
[F1] Log ProcessEvent
[F2] Stop logging ProcessEvent
[F3] Start logging functions that start with OnRep
[F4] Stop logging functions that start with OnRep
[F7] GUI
)");

    auto BaseAddr = (uintptr_t)GetModuleHandleA(0);

    if (!Setup(ProcessEventDetour))
    {
        MessageBoxA(0, _("Failed to setup!"), _("Error"), MB_ICONERROR);
        return 0;
    }
	
    std::cout << '\n' << Binds << _("\nSetup!\n");

    CreateThread(0, 0, Input, 0, 0, 0);

    CreateThread(0, 0, CreateGUI, 0, 0, 0);

    CreateThread(0, 0, Other, 0, 0, 0);

    Helper::Console::SpawnConsole();
    Helper::Console::SpawnCheatManager();

    auto LoadingScreenA = FindPattern(_("48 89 5C 24 ? 55 57 41 54 41 55 41 56 48 8B EC 48 83 EC 30 4C 8B E1 48 C7 45 ? ? ? ? ? 45 33 F6 48 8D 4D F0 33 D2 4C 89 75 F0 E8 ? ? ? ? 48"));
	
    if (!LoadingScreenA)
    {
        LoadingScreenA = FindPattern(_("48 89 4C 24 ? 55 53 56 57 41 56 41 57 48 8B EC 48 83 EC 38 48 8B F1 48 C7 45 ? ? ? ? ? 45 33 F6 48 8D 4D E8 33 D2 4C 89 75 E8 E8 ? ? ? ? 48 8B")); // s6
        
        if (!LoadingScreenA)
        {
            LoadingScreenA = FindPattern(_("48 89 5C 24 ? 48 89 4C 24 ? 55 56 57 41 54 41 56 48 8B EC 48 83 EC 30 48 8B F1 48 C7 45 ? ? ? ? ? 45 33 F6 48 8D 4D F0 33 D2 4C 89 75 F0")); // 5.41
            
            if (!LoadingScreenA)
            {
                LoadingScreenA = FindPattern(_("48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 33 ED 4C 8B F9 48 8D 4D B7 4C 89 6D B7 4C 89 6D BF 41 8D 55 34 E8 ? ? ? ? 8B 55 BF")); // 12.61
            
                if (!LoadingScreenA)
                {
                    std::cout << _("[WARNING] Unable to find IsShowingInitialLoadingScreen!") << '\n';
                }
            }
        }
    }

    auto ReplicateMoveToServerA = FindPattern(_("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 5B 20 48 8D 05 ? ? ? ? 49 89 73 F0 33 F6 40 38 35 ? ? ? ? 49 89 7B E8 49 8B F8 48 0F 45 C6 4D 89 73 D0 48 89 45 00 4C 8B F1"));

    if (!ReplicateMoveToServerA)
        std::cout << _("[WARNING] Unable to find ReplicateMoveToServer!") << '\n';
    else
    {
        ReplicateMoveToServer = decltype(ReplicateMoveToServer)(ReplicateMoveToServerA);

        MH_CreateHook((PVOID)ReplicateMoveToServerA, ReplicateMoveToServerDetour, (PVOID*)&ReplicateMoveToServer);
        MH_EnableHook((PVOID)ReplicateMoveToServerA);

        std::cout << _("Hooked ReplicateMoveToServer!\n");
    }

    auto GetPredictionData_Client_CharacterA = FindPattern(_("40 53 48 83 EC 20 48 8B 81 ? ? ? ? 48 8B D9 48 85 C0 75 14 48 81 C1 ? ? ? ? 48 8B 01 FF 50 28 48 89 83 ? ? ? ? 48 83 C4 20"));

    if (!GetPredictionData_Client_CharacterA)
        std::cout << _("[WARNING] Unable to find GetPredictionData_Client_Character!") << '\n';
    else
    {
        GetPredictionData_Client_Character = decltype(GetPredictionData_Client_Character)(GetPredictionData_Client_CharacterA);

        MH_CreateHook((PVOID)GetPredictionData_Client_CharacterA, GetPredictionData_Client_CharacterDetour, (PVOID*)&GetPredictionData_Client_Character);
        MH_EnableHook((PVOID)GetPredictionData_Client_CharacterA);

        std::cout << _("Hooked GetPredictionData_Client_Character!\n");
    }


    if (LoadingScreenA)
    {
        if (Engine_Version < 424)
        {
            IsShowingInitialLoadingScreen = decltype(IsShowingInitialLoadingScreen)(LoadingScreenA);

            MH_CreateHook((PVOID)LoadingScreenA, IsShowingInitialLoadingScreenDetour, (PVOID*)&IsShowingInitialLoadingScreen);
            MH_EnableHook((PVOID)LoadingScreenA);
        }
        else
        {
            IsShowingInitialLoadingScreenC2 = decltype(IsShowingInitialLoadingScreenC2)(LoadingScreenA);

            MH_CreateHook((PVOID)LoadingScreenA, IsShowingInitialLoadingScreenC2Detour, (PVOID*)&IsShowingInitialLoadingScreenC2);
            MH_EnableHook((PVOID)LoadingScreenA);
        }

        std::cout << _("Hooked IsShowingInitialLoadingScreen!\n");
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dllReason, LPVOID lpReserved)
{
    switch (dllReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, Main, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

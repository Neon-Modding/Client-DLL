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

    std::cout << _("Client DLL thing by Milxnor.\n");

    auto BaseAddr = (uintptr_t)GetModuleHandleA(0);

    if (!Setup(ProcessEventDetour))
    {
        MessageBoxA(0, _("Failed to setup!"), _("Error"), MB_ICONERROR);
        return 0;
    }
	
    std::cout << _("Setup!\n");

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
            std::cout << _("Warning, unable to find IsShowingInitialLoadingScreen!") << '\n';
        }
    }

    if (LoadingScreenA)
    {
        IsShowingInitialLoadingScreen = decltype(IsShowingInitialLoadingScreen)(LoadingScreenA);

        MH_CreateHook((PVOID)LoadingScreenA, IsShowingInitialLoadingScreenDetour, (PVOID*)&IsShowingInitialLoadingScreen);
        MH_EnableHook((PVOID)LoadingScreenA);
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

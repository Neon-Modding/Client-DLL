#pragma once

// TODO: Update ImGUI

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>
#include <Kiero/kiero.h>
#include <iostream>

#include "helper.h"
#include "detours.h"

HRESULT(WINAPI* PresentOriginal)(IDXGISwapChain* SwapChain, uint32_t Interval, uint32_t Flags);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND wnd = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

static bool bHasInit = false;
static bool bShow = false;

LRESULT __stdcall WndProc(const HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYUP && (wParam == VK_F7 || (bShow && wParam == VK_ESCAPE)))
	{
		bShow = !bShow;
		ImGui::GetIO().MouseDrawCursor = bShow;
	}
	else if (uMsg == WM_QUIT && bShow)
		ExitProcess(0);

	if (bShow)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return TRUE;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT WINAPI HookPresent(IDXGISwapChain* SwapChain, uint32_t Interval, uint32_t Flags)
{
	if (!bHasInit)
	{
		auto stat = SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)&pDevice));
		if (stat)
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			SwapChain->GetDesc(&sd);
			wnd = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

			ImGui_ImplWin32_Init(wnd);
			ImGui_ImplDX11_Init(pDevice, pContext);

			bHasInit = true;
		}

		else return PresentOriginal(SwapChain, Interval, Flags);
	}

	if (bShow) // TODO: Acutaly learn ImGUI and rewrite
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		ImGui::SetNextWindowBgAlpha(0.8f);
		ImGui::SetNextWindowSize(ImVec2(560, 345));

		ImGui::Begin(_("Client DLL"), 0, ImGuiWindowFlags_NoCollapse);

		// initialize variables used by the user using the gui

		static int Tab = 1;

		if (ImGui::BeginTabBar(_(""))) { // Figure out what tab they are on
			if (ImGui::BeginTabItem(_("Client")))
			{
				Tab = 1;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(_("Logging")))
			{
				Tab = 2;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(_("Developer")))
			{
				Tab = 3;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(_("Movement")))
			{
				Tab = 4;
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		switch (Tab) // now that we know what tab we can now display what that tab has
		{
		case 1:
			if (ImGui::Button(_("Print Roles")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				auto Pawn = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.Controller.Pawn"));

				if (PC)
				{
					std::cout << _("PlayerController Role: ") << Helper::RoleToString((ENetRole)(Helper::GetRole(PC).Get())) << '\n';
					std::cout << _("PlayerController RemoteRole: ") << Helper::RoleToString((ENetRole)Helper::GetRole(PC, true).Get()) << '\n';
				}

				else
					std::cout << _("Could not find PlayerController!\n");

				if (Pawn && *Pawn)
				{
					std::cout << _("Pawn Role: ") << Helper::RoleToString((ENetRole)Helper::GetRole(*Pawn).Get()) << '\n';
					std::cout << _("Pawn RemoteRole: ") << Helper::RoleToString((ENetRole)Helper::GetRole(*Pawn, true).Get()) << '\n';
				}

				else
					std::cout << _("Could not find Pawn!\n");
			}

			else if (ImGui::Button(_("Change PC & Pawn to Simulated")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				auto Pawn = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.Controller.Pawn"));

				if (PC)
				{
					std::string role;

					role = _("ByteProperty /Script/Engine.Actor.RemoteRole");

					*PC->Member<TEnumAsByte<ENetRole>>(role) = ENetRole::ROLE_SimulatedProxy;
					if (Pawn && *Pawn)
					{
						*(*Pawn)->Member<TEnumAsByte<ENetRole>>(role) = ENetRole::ROLE_SimulatedProxy;
						std::cout << "Changed Pawn Role!\n";
					}

					std::cout << "Changed PC Role!\n";
				}

				else
					std::cout << _("Could not find PlayerController!\n");
			}

			else if (ImGui::Button(_("Change PC & Pawn to Autonomous")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				auto Pawn = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.Controller.Pawn"));

				if (PC)
				{
					std::string role;

					role = _("ByteProperty /Script/Engine.Actor.RemoteRole");

					*PC->Member<TEnumAsByte<ENetRole>>(role) = ENetRole::ROLE_AutonomousProxy;
					if (Pawn && *Pawn)
					{
						*(*Pawn)->Member<TEnumAsByte<ENetRole>>(role) = ENetRole::ROLE_AutonomousProxy;
						std::cout << "Changed Pawn Role!\n";
					}

					std::cout << "Changed PC Role!\n";
				}

				else
					std::cout << _("Could not find PlayerController!\n");
			}

			else if (ImGui::Button("Print PlayerController"))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				std::cout << PC << '\n';

				if (PC)
				{
					std::cout << _("Printing out PlayerController name in 2 seconds...\n");

					Sleep(2000);

					std::cout << PC->GetFullName() << '\n';
				}

				else
					std::cout << _("Could not find PlayerController!");
			}

			else if (ImGui::Button(_("Print Pawn")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto Pawn = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.Controller.Pawn"));

					if (Pawn && *Pawn)
					{
						std::cout << *Pawn << '\n';

						std::cout << _("Printing out Pawn name in 2 seconds...\n");

						Sleep(2000);

						std::cout << (*Pawn)->GetFullName() << '\n';
					}

					else
						std::cout << _("Could not find Pawn!\n");

				}

				else
					std::cout << _("Could not find PlayerController!");
			}

			else if (ImGui::Button(_("Create CheatManager (Athena)")))
			{
				Helper::Console::SpawnCheatManager();
			}

			else if (ImGui::Button(_("Create CheatManager (GameModeBase)")))
			{
				Helper::Console::SpawnCheatManager(false);
			}

			else if (ImGui::Button(_("Enable LoadingScreen")))
			{
				bHideLoadingScreen = false;
				std::cout << _("Enabled Loading Screen!\n");
			}

			else if (ImGui::Button(_("Disable LoadingScreen")))
			{
				bHideLoadingScreen = true;

				std::cout << _("Disabled Loading Screen!\n");
			}

			else if (ImGui::Button(_("Enable Autosprint")))
			{
				if (bHasLoadingScreenDropped)
				{
					bAutoSprint = true;
					std::cout << _("Enabled Autosprint!\n");
				}
				else
					std::cout << _("Loading screen still hasn't dropped!\n");
			}

			else if (ImGui::Button(_("Disable Autosprint")))
			{
				if (bHasLoadingScreenDropped)
				{
					bAutoSprint = false;
					std::cout << _("Disabled Autosprint!\n");
				}
				else
					std::cout << _("Loading screen still hasn't dropped!\n");
			}

			break;
		case 2:
			if (ImGui::Button(_("Log ProcessEvent")))
			{
				bLogEverything = !bLogEverything;
			}

			else if (ImGui::Button(_("Log OnRep")))
			{
				if (!bLogRep)
					WriteToLog(_("\n\n\nNew\n"), _("OnRep_log"));

				bLogRep = !bLogRep;
			}

			else if (ImGui::Button(_("Log RPCS")))
			{
				if (!bLogRPCS)
					WriteToLog(_("\n\n\nNew\n"), _("RPCS_log"));

				bLogRPCS = !bLogRPCS;
			}

			else if (ImGui::Button(_("Print bHasFinishedLoading")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				bool bHasFinishedLoading = PC->Member<bool>(_("BoolProperty /Script/FortniteGame.FortPlayerState.bHasFinishedLoading"));

				std::cout << _("bHasFinishedLoading: ") << bHasFinishedLoading << '\n';
			}

			else if (ImGui::Button(_("Print bHasFinishedLoading (Pawn)")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				auto Pawn = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.Controller.Pawn"));

				bool bHasFinishedLoading = (*Pawn)->Member<bool>(_("BoolProperty /Script/FortniteGame.FortPlayerState.bHasFinishedLoading"));

				std::cout << _("bHasFinishedLoading: ") << bHasFinishedLoading << '\n';
			}

			else if (ImGui::Button(_("Print MatchState")))
			{
				auto World = *GetWorld();

				if (!World)
					std::cout << _("Could not find World!\n");

				auto GameMode = World->Member<UObject*>(_("ObjectProperty /Script/Engine.World.AuthorityGameMode"));

				if (GameMode && *GameMode)
				{
					auto GameState = (*GameMode)->Member<UObject*>(_("ObjectProperty /Script/Engine.GameModeBase.GameState"));
					auto State = (*(*GameState)->Member<FName>(_("NameProperty /Script/Engine.GameState.MatchState"))).ToString();
					std::cout << _("MatchState (GameMode): ") << State << '\n';
				}

				else
					std::cout << _("Could not find GameModeBase!\n");

				auto GameState2 = World->Member<UObject*>(_("ObjectProperty /Script/Engine.World.GameState"));

				if (GameState2 && *GameState2)
				{
					auto State2 = (*(*GameState2)->Member<FName>(_("NameProperty /Script/Engine.GameState.MatchState"))).ToString();

					std::cout << _("MatchState (World): ") << State2 << '\n';
				}

				else
					std::cout << _("Could not find GameState in World\n");
			}

			else if (ImGui::Button(_("Print bReadyToStartMatch")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				std::cout << _("bReadyToStartMatch: ") << *PC->Member<bool>(_("BoolProperty /Script/FortniteGame.FortPlayerController.bReadyToStartMatch")) << '\n';
			}

			else if (ImGui::Button(_("Set bReadyToStartMatch to true")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("GameInstance")))->Member<TArray<UObject*>>(_("LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				*PC->Member<bool>(_("bReadyToStartMatch")) = true;

				std::cout << _("bReadyToStartMatch was set to true!\n");
			}

			else if (ImGui::Button(_("Set State to InProgress")))
			{
				auto World = *GetWorld();

				if (!World)
					std::cout << _("Could not find World!\n");

				auto GameMode = World->Member<UObject*>(_("AuthorityGameMode"));

				if (GameMode && *GameMode)
				{
					auto GameState = (*GameMode)->Member<UObject*>(_("GameState"));
					auto State = (*(*GameState)->Member<FName>(_("MatchState"))).ToString();
					std::cout << _("MatchState (GameMode): ") << State << '\n';
				}

				else
					std::cout << _("Could not find GameModeBase!\n");

				auto GameState2 = World->Member<UObject*>(_("GameState"));

				if (GameState2 && *GameState2)
				{
					auto State2 = (*GameState2)->Member<FName>(_("MatchState"));

					FName name;
					name.Number = 0;
					name.ComparisonIndex = 12772;

					*State2 = name;

					std::cout << _("MatchState (World): ") << State2 << '\n';
				}

				else
					std::cout << _("Could not find GameState in World\n");
			}

			else if (ImGui::Button(_("Log Acknowledged Pawn")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto Pawn = PC->Member<UObject*>(_("AcknowledgedPawn"));
					if (Pawn && *Pawn)
					{
						std::cout << "AcknowledgedPawn: " << (*Pawn)->GetFullName() << '\n';
					}
					else
						std::cout << _("No AcknowledgedPawn!\n");
				}
			}

			else if (ImGui::Button(_("Call ServerShortTimeOut")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					PC->ProcessEvent(PC->Function(_("ServerShortTimeout")), nullptr);
					std::cout << _("Called ServerShortTimeout!\n");
				}
			}

			else if (ImGui::Button(_("Print CameraManager")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto CameraManager = PC->Member<UObject*>(_("PlayerCameraManagerClass"));

					if (CameraManager && *CameraManager)
						std::cout << _("Camera Manager: ") << (*CameraManager)->GetFullName() << '\n';
				}
			}
			/* else if (ImGui::Button(_("Print ItemInstances (3.5)")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto WorldInventory = *PC->Member<UObject*>(_("WorldInventory"));
					
					if (WorldInventory)
					{
						auto ItemInstances = (TArray<UObject*>*)(__int64(*WorldInventory->Member<__int64>(_("Inventory"))) + 0x110);

						if (ItemInstances)
						{
							std::cout << _("ItemInstances Num: ") << ItemInstances->Num() << '\n';
							for (int i = 0; i < ItemInstances->Num(); i++)
							{
								auto ItemInstance = ItemInstances->At(i);

								if (!ItemInstance)
									continue;

								std::cout << std::format("[{}] {}", i, ItemInstance->GetFullName()) << '\n';
							}
						}
						else
							std::cout << _("Could not find ItemInstances!\n");
					}
				}
			}

			else if (ImGui::Button(_("ItemInstances Test")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto WorldInventory = *PC->Member<UObject*>(_("WorldInventory"));

					if (WorldInventory)
					{
						std::cout << "ItemInstances Offsets: " << GetOffset(WorldInventory->Member<UObject>(_("Inventory")), _("ItemInstances")) << '\n';
					}
				}
			}

			else if (ImGui::Button(_("Print ReplicatedEntries (3.5)")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto WorldInventory = *PC->Member<UObject*>(_("WorldInventory"));

					if (WorldInventory)
					{
						auto ReplicatedEntries = (TArray<__int64>*)(__int64(*WorldInventory->Member<__int64>(_("Inventory"))) + 0xB0);

						if (ReplicatedEntries)
						{
							std::cout << _("ReplicatedEntries Num: ") << ReplicatedEntries->Num() << '\n';
							
							for (int i = 0; i < ReplicatedEntries->Num(); i++)
							{
								auto ReplicatedEntry = ReplicatedEntries->At(i);

								if (!ReplicatedEntry)
									continue;

								std::cout << std::format("[{}] {}", i, (*(UObject**)(__int64(ReplicatedEntry) + 0x18))->GetFullName()) << '\n';
							}
						}
						else
							std::cout << _("Could not find ReplicatedEntries!\n");
					}
				}
			} */

			break;
		case 3:
			if (ImGui::Button(_("Dump Objects")))
				CreateThread(0, 0, DumpObjects, 0, 0, 0);

			else if (ImGui::Button(_("Call OnRep_ReplicatedMovement")))
			{
				auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

				if (PC)
				{
					auto Pawn = PC->Member<UObject*>(_("Pawn"));
					if (Pawn && *Pawn)
					{
						static auto OnRep_ReplicatedMovement = (*Pawn)->Function(_("OnRep_ReplicatedMovement"));

						if (OnRep_ReplicatedMovement)
						{
							(*Pawn)->ProcessEvent(OnRep_ReplicatedMovement, nullptr);
							std::cout << _("Called OnRep_ReplicatedMovement!\n");
						}
					}
				}
			}

			break;
		case 4:
			if (ImGui::Button(_("Set bReplicateMovement to true")))
			{
				bSetReplicatemOvementTotrue = !bSetReplicatemOvementTotrue;
				std::cout << _("Set bReplicateMovement!\n");
			}
			break;
		}

		ImGui::End();
		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	return PresentOriginal(SwapChain, Interval, Flags);
}

DWORD WINAPI CreateGUI(LPVOID)
{
	bool bHooked = false;
	while (!bHooked)
	{
		auto status = kiero::init(kiero::RenderType::D3D11 /* Auto */);
		if (status == kiero::Status::Success)
		{
			kiero::bind(8, (PVOID*)&PresentOriginal, HookPresent);
			bHooked = true;
		}

		Sleep(100);
	}

	std::cout << _("Initialized GUI!\n");

	return 0;
}
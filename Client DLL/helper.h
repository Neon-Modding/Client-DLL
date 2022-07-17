#pragma once

#include <fstream>
#include <format>

#include "structs.h"
#include "xorstr.hpp"

struct SpawnParams
{
	UObject* ObjectClass;
	UObject* Outer;
	UObject* ReturnValue;
};

DWORD WINAPI DumpObjects(LPVOID)
{
	std::ofstream stream(_("Objects.txt"));
	// auto Base = (uintptr_t)GetModuleHandleW(0);
	for (int i = 0; i < (ObjObjects ? ObjObjects->Num() : OldObjects->Num()); i++)
	{
		auto Object = ObjObjects ? ObjObjects->GetObjectById(i) : OldObjects->GetObjectById(i);
		if (Object)
		{
			auto fullname = Object->GetFullName();
			stream << std::format("[{}] {}\n", i, fullname);
			/*
			// stream << std::format("[{}] {}\n", i, fullname);
			if (fullname.find(_("Property")) != std::string::npos)
				stream << std::format(_("[{}] {} Offset: {}\n"), i, fullname, ((UProperty*)Object)->Offset);
			// else if (fullname.starts_with("Function"))
			//   stream << std::format("[{}] {} 0x{}\n", i, fullname, __int64(((UFunction*)Object)->Func) - Base);
			else
				stream << std::format(_("[{}] {}\n"), i, fullname);
			*/
		}
	}

	std::cout << _("Finished dumping Objects!\n");

	stream.close();
	return 0;
}

UObject** GetWorld()
{
	auto Engine = FindObject(_("FortEngine_"));

	if (Engine)
	{
		auto GameViewport = *Engine->Member<UObject*>(_("ObjectProperty /Script/Engine.Engine.GameViewport"));
		auto World = GameViewport->Member<UObject*>(_("ObjectProperty /Script/Engine.GameViewportClient.World"));
		return World;
	}

	return nullptr;
}


namespace Easy
{
	template <typename T = UObject>
	T* SpawnObject(UObject* Object, UObject* Outer)
	{
		if (!Object || !Outer) return nullptr;

		SpawnParams params{};
		params.ObjectClass = Object;
		params.Outer = Outer;

		static auto GSC = FindObject(_("GameplayStatics /Script/Engine.Default__GameplayStatics"));

		GSC->ProcessEvent(FindObject(_("Function /Script/Engine.GameplayStatics.SpawnObject")), &params);

		return params.ReturnValue;
	}
}

namespace Helper
{
	namespace Console
	{
		UObject** SpawnCheatManager(bool bFortCheatManager = true)
		{
			auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("ObjectProperty /Script/Engine.GameEngine.GameInstance")))->Member<TArray<UObject*>>(_("ArrayProperty /Script/Engine.GameInstance.LocalPlayers")))->At(0)->Member<UObject*>(_("ObjectProperty /Script/Engine.Player.PlayerController"))));

			UObject* CheatManagerClass = nullptr;

			if (bFortCheatManager)
				CheatManagerClass = FindObject(_("Class /Script/FortniteGame.FortCheatManager"));

			else
				CheatManagerClass = FindObject(_("Class /Script/Engine.CheatManager"));

			auto CheatManager = PC->Member<UObject*>(_("ObjectProperty /Script/Engine.PlayerController.CheatManager"));

			*CheatManager = Easy::SpawnObject(CheatManagerClass, PC);

			return CheatManager;
		}

		UObject** SpawnConsole()
		{
			auto FortGameViewportClient = FindObject(_("FortGameViewportClient /Engine/Transient.FortEngine_"));

			while (!FortGameViewportClient)
				FortGameViewportClient = FindObject(_("FortGameViewportClient /Engine/Transient.FortEngine_"));

			auto ConsoleClass = FindObject(_("Class /Script/Engine.Console"));
			UObject** ViewportConsole = FortGameViewportClient->Member<UObject*>(_("ObjectProperty /Script/Engine.GameViewportClient.ViewportConsole"));

			if (!*ViewportConsole)
				*ViewportConsole = Easy::SpawnObject(ConsoleClass, FortGameViewportClient);

			return ViewportConsole;
		}
	}

	TEnumAsByte<ENetRole>& GetRole(UObject* Object, bool bRemote = false)
	{
		std::string role;

		if (bRemote)
			role = _("ByteProperty /Script/Engine.Actor.Role");
		else
			role = _("ByteProperty /Script/Engine.Actor.RemoteRole");

		return *Object->Member<TEnumAsByte<ENetRole>>(role);
	}

	std::string RoleToString(ENetRole Role)
	{
		switch (std::stoi(std::to_string((uint8_t)Role)))
		{
		case 0:
			return _("None");
		case 1:
			return _("SimulatedProxy");
		case 2:
			return _("AutonomousProxy");
		case 3:
			return _("Authority");
		case 4:
			return _("Max");
		default:
			return _("Invalid");
		}
	}
	
	void SetMovementMode(UObject* Character, EMovementMode mode)
	{
		static auto CharacterMovement = *Character->Member<UObject*>(_("CharacterMovement"));
		static auto fn = CharacterMovement->Function(_("SetMovementMode"));

		struct {
			EMovementMode NewMovementMode;
			char NewCustomMode;
		} params{};

		params.NewMovementMode = mode;
		params.NewCustomMode = false;

		Character->ProcessEvent(fn, &params);
	}
}
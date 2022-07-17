#pragma once

#include <fstream>

#include "structs.h"
#include "xorstr.hpp"

static bool bLogEverything = false;
static bool bLogRep = false;
static bool bLogRPCS = false;
static bool bAutoSprint = false;
static bool bHasLoadingScreenDropped = false;
static UObject* g_Pawn = nullptr;

void(__fastcall* ReplicateMoveToServer)(UObject* movementComponent, float a2, const FVector* a3);

namespace Time
{
	static uint64_t GetEpochTime() { return (uint64_t)duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
	static uint64_t GetEpochTimeInMilliseconds() { return (uint64_t)duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

	static const tm* GetCurrentLocalTime()
	{
		time_t now = time(0);
		tm Result;
		auto currentTime = localtime_s(&Result, &now);
		if (currentTime != 0)
		{
			std::cout << _("Couldn't not get current local time!\n");
			return nullptr;
		}
		return &Result;
	}
	static int GetCurrentYear() { return GetCurrentLocalTime()->tm_year + 1900; }
	static int GetCurrentMonth() { return GetCurrentLocalTime()->tm_mon + 1; }
	static int GetCurrentDay() { return GetCurrentLocalTime()->tm_mday; }
	static int GetCurrentHour() { return GetCurrentLocalTime()->tm_hour; }
	static int GetCurrentMinute() { return GetCurrentLocalTime()->tm_min; }
	static int GetCurrentSecond() { return GetCurrentLocalTime()->tm_sec; }
	static std::string GetFullTime() { return std::format("{}-{}-{} {}:{}", GetCurrentMonth(), GetCurrentDay(), GetCurrentYear(), GetCurrentHour(), GetCurrentMinute()); }
};

void WriteToLog(std::string msg, const std::string& filename = _("Client_log"), bool bLogTime = true)
{
	if (bLogTime)
		msg = std::format("[{}] {}", Time::GetFullTime(), msg);

	std::ofstream f;
	f.open(filename + ".txt", std::ios::out | std::ios::app);
	f << msg << '\n';
	f.close();
}

void* ProcessEventDetour(UObject* Object, UObject* Function, void* Params)
{
	if (Object && Function)
	{
		auto ObjectName = Object->GetFullName();
		auto FunctionName = Function->GetFullName();

		if (bLogEverything)
		{
			if (!strstr(FunctionName.c_str(), _("EvaluateGraphExposedInputs")) &&
				!strstr(FunctionName.c_str(), _("Tick")) &&
				!strstr(FunctionName.c_str(), _("OnSubmixEnvelope")) &&
				!strstr(FunctionName.c_str(), _("OnSubmixSpectralAnalysis")) &&
				!strstr(FunctionName.c_str(), _("OnMouse")) &&
				!strstr(FunctionName.c_str(), _("Pulse")) &&
				!strstr(FunctionName.c_str(), _("BlueprintUpdateAnimation")) &&
				!strstr(FunctionName.c_str(), _("BlueprintPostEvaluateAnimation")) &&
				!strstr(FunctionName.c_str(), _("BlueprintModifyCamera")) &&
				!strstr(FunctionName.c_str(), _("BlueprintModifyPostProcess")) &&
				!strstr(FunctionName.c_str(), _("Loop Animation Curve")) &&
				!strstr(FunctionName.c_str(), _("UpdateTime")) &&
				!strstr(FunctionName.c_str(), _("GetMutatorByClass")) &&
				!strstr(FunctionName.c_str(), _("UpdatePreviousPositionAndVelocity")) &&
				!strstr(FunctionName.c_str(), _("IsCachedIsProjectileWeapon")) &&
				!strstr(FunctionName.c_str(), _("LockOn")) &&
				!strstr(FunctionName.c_str(), _("GetAbilityTargetingLevel")) &&
				!strstr(FunctionName.c_str(), _("ReadyToEndMatch")) &&
				!strstr(FunctionName.c_str(), _("ReceiveDrawHUD")) &&
				!strstr(FunctionName.c_str(), _("OnUpdateDirectionalLightForTimeOfDay")) &&
				!strstr(FunctionName.c_str(), _("GetSubtitleVisibility")) &&
				!strstr(FunctionName.c_str(), _("GetValue")) &&
				!strstr(FunctionName.c_str(), _("InputAxisKeyEvent")) &&
				!strstr(FunctionName.c_str(), _("ServerTouchActiveTime")) &&
				!strstr(FunctionName.c_str(), _("SM_IceCube_Blueprint_C")) &&
				!strstr(FunctionName.c_str(), _("OnHovered")) &&
				!strstr(FunctionName.c_str(), _("OnCurrentTextStyleChanged")) &&
				!strstr(FunctionName.c_str(), _("OnButtonHovered")) &&
				!strstr(FunctionName.c_str(), _("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")) && 
				!strstr(FunctionName.c_str(), _("UpdateCamera")) &&
				!strstr(FunctionName.c_str(), _("GetMutatorContext")) &&
				!strstr(FunctionName.c_str(), _("CanJumpInternal")))
			{
				// std::cout << FunctionName << ' ' << ObjectName << '\n';
				WriteToLog(FunctionName + ' ' + ObjectName);
			}
		}

		if (bLogRep)
		{
			if (FunctionName.contains(_("OnRep_")))
			{
				if (!FunctionName.contains(_("OnRep_ReplicatedMovement")) &&
					!FunctionName.contains(_("OnRep_AccelerationPack")) &&
					!FunctionName.contains(_("OnRep_AttachmentReplication")))
				{
					// std::cout << _("OnRep called!: ") << FunctionName << '\n';
					WriteToLog(FunctionName + ' ' + ObjectName, _("OnRep_log"));
				}
			}
		}

		if (bLogRPCS)
		{
			// if (Function->FunctionFlags & 0x00200000 || (Function->FunctionFlags & 0x01000000 && FunctionName.find("Ack") == -1 && FunctionName.find("AdjustPos") == -1))
			auto ShortName = Function->GetName();

			if (ShortName.starts_with(_("Client")) || ShortName.starts_with(_("Server")))
				WriteToLog(_("Client function called!: ") + FunctionName, _("RPCS_log"));
		}

		if (FunctionName.contains(_("UAC")))
		{
			WriteToLog(_("UAC function called!: ") + FunctionName, _("UAC_log"));
			return nullptr;
		}
		
		else if (FunctionName.contains(_("ServerLoadingScreenDropped")))
		{
			/* if (!bHasLoadingScreenDropped)
			{
				static auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("GameInstance")))->Member<TArray<UObject*>>(_("LocalPlayers")))->At(0)->Member<UObject*>(_("PlayerController"))));

				if (PC)
				{
					g_Pawn = *PC->Member<UObject*>(_("Pawn"));

					if (g_Pawn)
						bHasLoadingScreenDropped = true;
					else
						bHasLoadingScreenDropped = false;
				}
			}
			else
				bHasLoadingScreenDropped = false; */
		}

		else if (FunctionName.contains(_("ClientUpdatePositionAfterServerUpdate")))
		{
			std::cout << _("ClientUpdatePositionAfterServerUpdate! (This means the Client knows that we are actually a Client)");
		}

		if (bAutoSprint)
		{
			static auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("GameInstance")))->Member<TArray<UObject*>>(_("LocalPlayers")))->At(0)->Member<UObject*>(_("PlayerController"))));

			auto WantsToSprint = *PC->Member<bool>(_("bWantsToSprint"));
			*g_Pawn->Member<EFortMovementStyle>(_("CurrentMovementStyle")) =  WantsToSprint ? EFortMovementStyle::Sprinting : EFortMovementStyle::Sprinting;
		}
	}

	return ProcessEventO(Object, Function, Params);
}

DWORD WINAPI Other(LPVOID) // death bruh
{
	while (1)
	{
		static auto PC = (*(((*FindObject(_("FortEngine_"))->Member<UObject*>(_("GameInstance")))->Member<TArray<UObject*>>(_("LocalPlayers")))->At(0)->Member<UObject*>(_("PlayerController"))));

		if (g_Pawn && PC)
		{
			auto WantsToSprint = *PC->Member<bool>(_("bWantsToSprint"));
			auto CurrentMovementStyle = g_Pawn->Member<EFortMovementStyle>(_("CurrentMovementStyle"));
			
			if (*CurrentMovementStyle != EFortMovementStyle::Sprinting && *CurrentMovementStyle != EFortMovementStyle::Running)
				*g_Pawn->Member<EFortMovementStyle>(_("CurrentMovementStyle")) = WantsToSprint ? EFortMovementStyle::Sprinting : EFortMovementStyle::Walking;
		}

		Sleep(1000 / 30);
	}
}

char (*IsShowingInitialLoadingScreen)(__int64, __int64* a2);

static bool bHideLoadingScreen = false;

char __fastcall IsShowingInitialLoadingScreenDetour(__int64 a1, __int64* a2)
{
	if (bHideLoadingScreen)
		return 0;

	return IsShowingInitialLoadingScreen(a1, a2);
}

char (*IsShowingInitialLoadingScreenC2)(__int64);

char __fastcall IsShowingInitialLoadingScreenC2Detour(__int64 a1)
{
	if (bHideLoadingScreen)
		return 0;

	return IsShowingInitialLoadingScreenC2(a1);
}

// I hate movement.

void __fastcall ReplicateMoveToServerDetour(UObject* movementComponent, float a2, const FVector* a3)
{
	std::cout << _("ReplicateMoveToServer was called!\n");

	return ReplicateMoveToServer(movementComponent, a2, a3);
}

void* (__fastcall* GetPredictionData_Client_Character)(UObject* component);

bool bSetReplicatemOvementTotrue = false;

static bool GetBit(uint8_t b, int bitNumber) {
	return (b & (1 << bitNumber)) != 0;
}

void* __fastcall GetPredictionData_Client_CharacterDetour(UObject* component)
{
	std::cout << _("GetPredictionData_Client_Character was called!\n");

	auto CharacterOwner = component->Member<UObject*>(_("CharacterOwner"));

	if (CharacterOwner && *CharacterOwner)
	{
		std::cout << _("CharacterOwner: ") << CharacterOwner << '\n';

		auto bReplicateMovement = (*CharacterOwner)->Member<uint8_t>(_("bReplicateMovement"));

		std::cout << _("bReplicateMovement: ") << GetBit(*bReplicateMovement, (int)bReplicateMovement[0]) << '\n';
		/* if ()
		bReplicateMovement[0] = bSetReplicatemOvementTotrue;
		std::cout << _("Ahh: ") << bSetReplicatemOvementTotrue << '\n'; */
	}
	else
		std::cout << _("Invalid CharacterOwner: ") << CharacterOwner << '\n';

	return GetPredictionData_Client_Character(component);
}
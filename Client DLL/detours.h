#pragma once

#include <fstream>

#include "structs.h"
#include "xorstr.hpp"

static bool bLogEverything = false;
static bool bLogRep = false;
static bool bLogClient = false;

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
				!strstr(FunctionName.c_str(), _("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")))
			{
				std::cout << FunctionName << ' ' << ObjectName << '\n';
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
					std::cout << _("OnRep called!: ") << FunctionName << '\n';
					WriteToLog(FunctionName + ' ' + ObjectName, _("OnRep_log"));
				}
			}
		}

		if (bLogClient)
		{
			if (Function->GetName().starts_with(_("Client")))
			{
				WriteToLog(_("Client function called!: ") + FunctionName, _("ClientDLL_log"));
			}
		}

		if (FunctionName.contains(_("ReadyToStartMatch")))
		{
			std::cout << FunctionName << _(" called with ") << ObjectName << '\n';
		}
	}

	return ProcessEventO(Object, Function, Params);
}

char (*IsShowingInitialLoadingScreen)(__int64, __int64* a2);

static bool bHideLoadingScreen = false;

char __fastcall IsShowingInitialLoadingScreenDetour(__int64 a1, __int64* a2)
{
	if (bHideLoadingScreen)
		return 0;

	return IsShowingInitialLoadingScreen(a1, a2);
}
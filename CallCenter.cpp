#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include "CallCenter.h"

void CallCenter::Init()
{
		YRCallback::MouseEvent		=	Ares::MouseEvent;
		YRCallback::CmdLineParse	=	Ares::CmdLineParse;
		YRCallback::ExeRun			=	Ares::ExeRun;
		YRCallback::ExeTerminate	=	Ares::ExeTerminate;
		YRCallback::PostGameInit	=	Ares::PostGameInit;

		BuildingTypeClassCallback::Create		=	Foundations::Defaults;
		BuildingTypeClassCallback::Delete		=	Foundations::Defaults;
		BuildingTypeClassCallback::Load			=	Foundations::Load;
		BuildingTypeClassCallback::Save			=	Foundations::Save;
		BuildingTypeClassCallback::LoadFromINI	=	Foundations::LoadFromINI;

		HouseTypeClassCallback::Create		=	Countries::Construct;
		HouseTypeClassCallback::LoadFromINI	=	Countries::LoadFromINI;

		CommandClassCallback::Register	=	Ares::RegisterCommands;

	BIND_CALLBACKS(WeaponTypeClass);

	BIND_CALLBACKS(WarheadTypeClass);

	BIND_CALLBACKS(SuperWeaponTypeClass);

	SuperClassCallback::Launch = SuperWeaponTypeClassExt::SuperClass_Launch;

}

#include "CallCenter.h"

void CallCenter::Init()
{
		YRCallback::MouseEvent		=	Ares::MouseEvent;
		YRCallback::CmdLineParse	=	Ares::CmdLineParse;
		YRCallback::ExeRun			=	Ares::ExeRun;
		YRCallback::ExeTerminate	=	Ares::ExeTerminate;
		YRCallback::PostGameInit	=	Ares::PostGameInit;

		CommandClassCallback::Register	=	Ares::RegisterCommands;

	BIND_CALLBACKS(WeaponTypeClass);
	BIND_INI_CALLBACKS(WeaponTypeClass);

	BIND_CALLBACKS(WarheadTypeClass);
	BIND_INI_CALLBACKS(WarheadTypeClass);

	BIND_CALLBACKS(SuperWeaponTypeClass);
	BIND_INI_CALLBACKS(SuperWeaponTypeClass);

	SuperClassCallback::Launch = SuperWeaponTypeClassExt::SuperClass_Launch;

	BIND_CALLBACKS(TechnoTypeClass);
	BIND_INI_CALLBACKS(TechnoTypeClass);

	BIND_CALLBACKS(TechnoClass);

	BIND_CALLBACKS(HouseClass);
}

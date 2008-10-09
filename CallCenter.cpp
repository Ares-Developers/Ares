#include "CallCenter.h"

void CallCenter::Init()
{
		YRCallback::MouseEvent		=	Ares::MouseEvent;
		YRCallback::CmdLineParse	=	Ares::CmdLineParse;
		YRCallback::ExeRun			=	Ares::ExeRun;
		YRCallback::ExeTerminate	=	Ares::ExeTerminate;
		YRCallback::PostGameInit	=	Ares::PostGameInit;

		BuildingTypeClassCallback::Create		=	BuildingTypeClassExt::Defaults;
		BuildingTypeClassCallback::Delete		=	BuildingTypeClassExt::Defaults;
		BuildingTypeClassCallback::Load			=	BuildingTypeClassExt::Load;
		BuildingTypeClassCallback::Save			=	BuildingTypeClassExt::Save;
		BuildingTypeClassCallback::LoadFromINI	=	BuildingTypeClassExt::LoadFromINI;

		HouseTypeClassCallback::Create		=	Countries::Construct;
		HouseTypeClassCallback::LoadFromINI	=	Countries::LoadFromINI;

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

	RulesClassCallback::Addition = RulesClassExt::Addition;
	RulesClassCallback::TypeData = RulesClassExt::TypeData;
}

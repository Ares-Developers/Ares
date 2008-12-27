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

	/* explicitly declaring callbacks for injgen - don't move, indent, or remove */

DEFINE_HOOK_AGAIN(41C9E5, _AircraftTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(41CE81, _AircraftTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(41CEAA, _AircraftTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(41CFE0, _AircraftTypeClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(45E50F, _BuildingTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(45E580, _BuildingTypeClassCallback_Delete, 5)
DEFINE_HOOK_AGAIN(464A47, _BuildingTypeClassCallback_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(4652ED, _BuildingTypeClassCallback_Load, 7)
DEFINE_HOOK_AGAIN(46536E, _BuildingTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(4F6531, _HouseClassCallback_Create, 6)
DEFINE_HOOK_AGAIN(4F7140, _HouseClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(504070, _HouseClassCallback_Load, 5)
DEFINE_HOOK_AGAIN(5046E2, _HouseClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(511635, _HouseTypeClassCallback_Create, 5)
DEFINE_HOOK_AGAIN(511645, _HouseTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(51214F, _HouseTypeClassCallback_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(512472, _HouseTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(51255E, _HouseTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(512760, _HouseTypeClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(523976, _InfantryTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(524B5A, _InfantryTypeClassCallback_Load, 4)
DEFINE_HOOK_AGAIN(524C54, _InfantryTypeClassCallback_Save, 5)
DEFINE_HOOK_AGAIN(524D70, _InfantryTypeClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(5F7314, _ObjectTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(5F7400, _ObjectTypeClassCallback_Delete, 5)
DEFINE_HOOK_AGAIN(5F9948, _ObjectTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(5F996A, _ObjectTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(668F6A, _RulesClassCallback_Addition, 5)
DEFINE_HOOK_AGAIN(675205, _RulesClassCallback_Save, 6)
DEFINE_HOOK_AGAIN(678841, _RulesClassCallback_Load, 7)
DEFINE_HOOK_AGAIN(679A10, _RulesClassCallback_TypeData, 5)
DEFINE_HOOK_AGAIN(6A460B, _SideClassCallback_Create, 5)
DEFINE_HOOK_AGAIN(6A4610, _SideClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(6A488E, _SideClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(6A48FE, _SideClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(6CC3A3, _SuperClassCallback_Launch, 6)
DEFINE_HOOK_AGAIN(6CE6F8, _SuperWeaponTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(6CE8C5, _SuperWeaponTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(6CE8EA, _SuperWeaponTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(6CEFE0, _SuperWeaponTypeClassCallback_Delete, 8)
DEFINE_HOOK_AGAIN(6F3262, _TechnoClassCallback_Create, 7)
DEFINE_HOOK_AGAIN(6F4500, _TechnoClassCallback_Delete, 5)
DEFINE_HOOK_AGAIN(70C24B, _TechnoClassCallback_Load, 5)
DEFINE_HOOK_AGAIN(70C266, _TechnoClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(71183B, _TechnoTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(711AE0, _TechnoTypeClassCallback_Delete, 5)
DEFINE_HOOK_AGAIN(716DB6, _TechnoTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(717096, _TechnoTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(7472B4, _UnitTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(7480A7, _UnitTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(7480CA, _UnitTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(748190, _UnitTypeClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(75D1AD, _WarheadTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(75E2B2, _WarheadTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(75E39E, _WarheadTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(75E510, _WarheadTypeClassCallback_Delete, 6)
DEFINE_HOOK_AGAIN(771EEE, _WeaponTypeClassCallback_Create, 3)
DEFINE_HOOK_AGAIN(772EA9, _WeaponTypeClassCallback_Load, 3)
DEFINE_HOOK_AGAIN(772F8E, _WeaponTypeClassCallback_Save, 3)
DEFINE_HOOK_AGAIN(7730F0, _WeaponTypeClassCallback_Delete, 7)

DEFINE_HOOK_AGAIN(533058, _CommandClassCallback_Register, 0)
DEFINE_HOOK_AGAIN(4AAC17, _YRCallback_MouseEvent, 5)
DEFINE_HOOK_AGAIN(52F639, _YRCallback_CmdLineParse, 5)
DEFINE_HOOK_AGAIN(7CD810, _YRCallback_ExeRun, 9)
DEFINE_HOOK_AGAIN(7CD8EF, _YRCallback_ExeTerminate, 9)


}

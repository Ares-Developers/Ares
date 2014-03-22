#include "Ares.h"

#include "Ext/Abstract/Body.h"
#include "Ext/AnimType/Body.h"
#include "Ext/Building/Body.h"
#include "Ext/BuildingType/Body.h"
#include "Ext/Bullet/Body.h"
#include "Ext/BulletType/Body.h"
#include "Ext/House/Body.h"
#include "Ext/HouseType/Body.h"
#include "Ext/Infantry/Body.h"
#include "Ext/Rules/Body.h"
#include "Ext/Side/Body.h"
#include "Ext/SWType/Body.h"
#include "Ext/TAction/Body.h"
#include "Ext/Techno/Body.h"
#include "Ext/TechnoType/Body.h"
#include "Ext/TEvent/Body.h"
#include "Ext/Tiberium/Body.h"
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

#include "Enum/ArmorTypes.h"

DEFINE_HOOK(7258D0, AnnounceInvalidPointer, 6)
{
	GET(void*, ptr, ECX);
	GET(bool, bRemoved, EDX);

	if(Ares::bShuttingDown) {
		return 0;
	}

	//Debug::Log("PointerGotInvalid: %X\n", ptr);

	AbstractExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	AnimTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	BuildingExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	BuildingTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	BulletExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	BulletTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	HouseExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	HouseTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	InfantryExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	SideExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	SWTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	TActionExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	TechnoExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	TechnoTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	TEventExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	TiberiumExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	WarheadTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);
	WeaponTypeExt::ExtMap.PointerGotInvalid(ptr, bRemoved);

	RulesExt::Global()->InvalidatePointer(ptr, bRemoved);

	return 0;
}

DEFINE_HOOK(685659, Scenario_ClearClasses, a)
{
	AbstractExt::ExtMap.Clear();
	AnimTypeExt::ExtMap.Clear();
	BuildingExt::ExtMap.Clear();
	BuildingExt::Cleanup();
	BuildingTypeExt::ExtMap.Clear();
	BulletExt::ExtMap.Clear();
	BulletTypeExt::ExtMap.Clear();
	HouseExt::ExtMap.Clear();
	HouseTypeExt::ExtMap.Clear();
	InfantryExt::ExtMap.Clear();
	SideExt::ExtMap.Clear();
	SWTypeExt::ExtMap.Clear();
	TActionExt::ExtMap.Clear();
	TechnoExt::ExtMap.Clear();
	TechnoTypeExt::ExtMap.Clear();
	TEventExt::ExtMap.Clear();
	TiberiumExt::ExtMap.Clear();
	WarheadTypeExt::ExtMap.Clear();
	WeaponTypeExt::ExtMap.Clear();

	ArmorType::ClearArray();
	RadType::ClearArray();
	GenericPrerequisite::ClearArray();

	RulesExt::ClearCameos();	
	RulesExt::Allocate(RulesClass::Instance);

	return 0;
}

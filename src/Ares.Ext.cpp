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
	GET(void *, DEATH, ECX);
	GET(bool, bRemoved, EDX);

	//Debug::Log("PointerGotInvalid: %X\n", DEATH);

	AbstractExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	AnimTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	BuildingExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	BuildingTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	BulletExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	BulletTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	HouseExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	HouseTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	InfantryExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	SideExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	SWTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	TActionExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	TechnoExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	TechnoTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	TEventExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	TiberiumExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	WarheadTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);
	WeaponTypeExt::ExtMap.PointerGotInvalid(DEATH, bRemoved);

	RulesExt::Global()->InvalidatePointer(DEATH, bRemoved);

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

	RulesExt::ClearCameos();

	ArmorType::ClearArray();
	RadType::ClearArray();
	GenericPrerequisite::ClearArray();

	return 0;
}

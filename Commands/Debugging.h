#ifndef CMD_DEBUGGING_H
#define CMD_DEBUGGING_H

#include "Ares.h"
#include "../Misc/Debug.h"

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/House/Body.h"
#include "../Ext/HouseType/Body.h"
#include "../Ext/WeaponType/Body.h"
#include "../Ext/WarheadType/Body.h"

#include <YRPP.h>

class DebuggingCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~DebuggingCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "Debug Dump"; }

	virtual const wchar_t* GetUIName()
		{ return L"Debugging Dump"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current debug data to the log"; }

	virtual void Execute(DWORD dwUnk)
	{
#define ARRAY_SIZE(t) \
		Debug::Log("Size of " str(t) "::Array is: %d items * %d bytes = %d bytes total\n", t::Array->Count , sizeof(t), t::Array->Count * sizeof(t));

		Debug::Log("Reporting main array sizes\n");

		ARRAY_SIZE(AnimClass);
		ARRAY_SIZE(AnimTypeClass);
		ARRAY_SIZE(HouseClass);
		ARRAY_SIZE(HouseTypeClass);
		ARRAY_SIZE(SideClass);
		ARRAY_SIZE(OverlayClass);
		ARRAY_SIZE(OverlayTypeClass);
		ARRAY_SIZE(ParticleClass);
		ARRAY_SIZE(ParticleTypeClass);
		ARRAY_SIZE(ParticleSystemClass);
		ARRAY_SIZE(ParticleSystemTypeClass);
//		ARRAY_SIZE(SuperClass);
		ARRAY_SIZE(SuperWeaponTypeClass);
		ARRAY_SIZE(WarheadTypeClass);
		ARRAY_SIZE(WeaponTypeClass);
		ARRAY_SIZE(BulletClass);
		ARRAY_SIZE(BulletTypeClass);

		ARRAY_SIZE(TechnoClass);
		ARRAY_SIZE(TechnoTypeClass);
		ARRAY_SIZE(AircraftClass);
		ARRAY_SIZE(AircraftTypeClass);
		ARRAY_SIZE(InfantryClass);
		ARRAY_SIZE(InfantryTypeClass);
		ARRAY_SIZE(UnitClass);
		ARRAY_SIZE(UnitTypeClass);
		ARRAY_SIZE(BuildingClass);
		ARRAY_SIZE(BuildingTypeClass);

//		ARRAY_SIZE(ScriptClass);
		ARRAY_SIZE(ScriptTypeClass);
		ARRAY_SIZE(TeamClass);
		ARRAY_SIZE(TeamTypeClass);
		ARRAY_SIZE(TaskForceClass);
		ARRAY_SIZE(AITriggerTypeClass);

//		ARRAY_SIZE(TActionClass);
//		ARRAY_SIZE(TEventClass);
//		ARRAY_SIZE(TriggerClass);
//		ARRAY_SIZE(TriggerTypeClass);

		ARRAY_SIZE(WaveClass);
//		ARRAY_SIZE(VoxelAnimClass);
//		ARRAY_SIZE(VoxelAnimTypeClass);
//		ARRAY_SIZE(CaptureManagerClass);
//		ARRAY_SIZE(SpawnManagerClass);
//		ARRAY_SIZE(SlaveManagerClass);
		ARRAY_SIZE(ParasiteClass);
//		ARRAY_SIZE(BombClass);

		for(MixFileClass *MIX = reinterpret_cast<MixFileClass *>(MixFileClass::MIXes->First.Next); MIX; MIX = reinterpret_cast<MixFileClass *>(MIX->Next)) {
			Debug::Log("MIX %s - %d bytes\n", MIX->FileName, MIX->FileSize);
		}

		Debug::Log("Done reporting sizes.\n");
		MessageListClass::PrintMessage(L"Array sizes logged");
	}

	//Constructor
	DebuggingCommandClass(){}
};

#endif

#ifndef CMD_DUMPTYPES_H
#define CMD_DUMPTYPES_H

#include "../Ares.h"
#include "../Misc/Debug.h"

class DumperTypesCommandClass : public AresCommandClass
{
public:
	//Destructor
	virtual ~DumperTypesCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Dump Data Types"; }

	virtual const wchar_t* GetUIName()
	{ return L"Dump Types"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current type list to the log"; }

	template <typename T>
	void LogType(const char* pSection) const {
		Debug::Log("[%s]\n", pSection);

		int i = 0;
		for(auto pItem : *T::Array) {
			Debug::Log("%d = %s\n", i++, pItem->get_ID());
		}
	}

	virtual void Execute(DWORD dwUnk)
	{
		if(this->CheckDebugDeactivated()) {
			return;
		}

		Debug::Log("Dumping all Types\n\n");

		Debug::Log("Dumping Rules Types\n\n");

		LogType<AnimTypeClass>("Animations");
		LogType<WeaponTypeClass>("WeaponTypes");
		LogType<WarheadTypeClass>("Warheads");
		LogType<BulletTypeClass>("Projectiles");

		LogType<HouseTypeClass>("Countries");

		LogType<InfantryTypeClass>("InfantryTypes");
		LogType<UnitTypeClass>("VehicleTypes");
		LogType<AircraftTypeClass>("AircraftTypes");
		LogType<BuildingTypeClass>("BuildingTypes");

		LogType<SuperWeaponTypeClass>("SuperWeaponTypes");
		LogType<SmudgeTypeClass>("SmudgeTypes");
		LogType<OverlayTypeClass>("OverlayTypes");
//		LogType<TerrainTypeClass>("TerrainTypes"); // needs class map in YRPP
		LogType<ParticleTypeClass>("Particles");
		LogType<ParticleSystemTypeClass>("ParticleSystems");

/*
		Debug::Log("Dumping Art Types\n\n");
		Debug::Log("[Movies]\n");
		for(int i = 0; i < MovieInfo::Array->Count; ++i) {
			Debug::Log("%d = %s\n", i, MovieInfo::Array->GetItem(i).Name);
		}
*/

		Debug::Log("Dumping AI Types\n\n");
		LogType<ScriptTypeClass>("ScriptTypes");
		LogType<TeamTypeClass>("TeamTypes");
		LogType<TaskForceClass>("TaskForces");

		Debug::Log("[AITriggerTypes]\n");
		for(int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
			char Buffer[1024];
			AITriggerTypeClass::Array->Items[i]->FormatForSaving(Buffer, sizeof(Buffer));
			Debug::Log("%s\n", Buffer);
		}

		Debug::Log("[AITriggerTypesEnable]\n");
		for(int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
			AITriggerTypeClass *X = AITriggerTypeClass::Array->Items[i];
			Debug::Log("%X = %s\n", X->get_ID(), X->IsEnabled ? "yes" : "no");
		}

		MessageListClass::Instance->PrintMessage(L"Type data dumped");
	}

	//Constructor
	DumperTypesCommandClass(){}
};

#endif

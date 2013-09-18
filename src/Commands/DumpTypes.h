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

#define LOGTYPE(typestr, section) \
	Debug::Log("[" # section "]\n"); \
	for(int i = 0; i < typestr ## TypeClass::Array->Count; ++i) { \
		typestr ## TypeClass *X = typestr ## TypeClass::Array->GetItem(i); \
		Debug::Log("%d = %s\n", i, X->get_ID()); \
	}

	virtual void Execute(DWORD dwUnk)
	{
		if(this->CheckDebugDeactivated()) {
			return;
		}

		Debug::Log("Dumping all Types\n\n");

		Debug::Log("Dumping Rules Types\n\n");

		LOGTYPE(Anim, Animations);
		LOGTYPE(Weapon, WeaponTypes);
		LOGTYPE(Warhead, Warheads);
		LOGTYPE(Bullet, Projectiles);

		LOGTYPE(House, Countries);

		LOGTYPE(Infantry, InfantryTypes);
		LOGTYPE(Unit, VehicleTypes);
		LOGTYPE(Aircraft, AircraftTypes);
		LOGTYPE(Building, BuildingTypes);

		LOGTYPE(SuperWeapon, SuperWeaponTypes);
		LOGTYPE(Smudge, SmudgeTypes);
		LOGTYPE(Overlay, OverlayTypes);
//		LOGTYPE(Terrain, TerrainTypes); // needs class map in YRPP
		LOGTYPE(Particle, Particles);
		LOGTYPE(ParticleSystem, ParticleSystems);

/*
		Debug::Log("Dumping Art Types\n\n");
		Debug::Log("[Movies]\n");
		for(int i = 0; i < MovieInfo::Array->Count; ++i) {
			Debug::Log("%d = %s\n", i, MovieInfo::Array->GetItem(i).Name);
		}
*/

		Debug::Log("Dumping AI Types\n\n");
		LOGTYPE(Script, ScriptTypes);
		LOGTYPE(Team, TeamTypes);

		Debug::Log("[TaskForces]\n");
		for(int i = 0; i < TaskForceClass::Array->Count; ++i) {
			TaskForceClass *X = TaskForceClass::Array->GetItem(i);
			Debug::Log("%d = %s\n", i, X->get_ID());
		}

		Debug::Log("[AITriggerTypes]\n");
		for(int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
			char Buffer[1024];
			memset(Buffer, 0, 1024);
			AITriggerTypeClass::Array->GetItem(i)->FormatForSaving(Buffer, sizeof(Buffer));
			Debug::Log("%s\n", Buffer);
		}

		Debug::Log("[AITriggerTypesEnable]\n");
		for(int i = 0; i < AITriggerTypeClass::Array->Count; ++i) {
			AITriggerTypeClass *X = AITriggerTypeClass::Array->GetItem(i);
			Debug::Log("%X = %s\n", X->get_ID(), X->IsEnabled ? "yes" : "no");
		}

		MessageListClass::Instance->PrintMessage(L"Type data dumped");
	}

	//Constructor
	DumperTypesCommandClass(){}
};

#endif

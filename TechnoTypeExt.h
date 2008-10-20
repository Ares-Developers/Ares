#ifndef TECHNOTYPE_EXT_H
#define TECHNOTYPE_EXT_H

#include <hash_map>
#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <TechnoTypeClass.h>

class TechnoTypeClassExt
{
	public:
	struct TechnoTypeClassData
	{
		DynamicVectorClass<InfantryTypeClass *> Survivors_Pilots;
		int Survivors_PilotChance;
		int Survivors_PassengerChance;

		// animated cameos
//		int Cameo_Interval;
//		int Cameo_CurrentFrame;
//		TimerStruct Cameo_Timer;

		DynamicVectorClass< DynamicVectorClass<int>* > PrerequisiteLists;
		DynamicVectorClass<int> PrerequisiteNegatives;
		DWORD PrerequisiteTheaters;

		// new secret lab
		DWORD Secret_RequiredHouses;
		DWORD Secret_ForbiddenHouses;

		bool Is_Deso;
		bool Is_Deso_Radiation;
		bool Is_Cow;

		DynamicVectorClass<WeaponStruct> Weapons;
		DynamicVectorClass<WeaponStruct> EliteWeapons;

		bool Data_Initialized;
		void Initialize(TechnoTypeClass *pThis);
	};

	EXT_P_DECLARE(TechnoTypeClass);
	EXT_FUNCS(TechnoTypeClass);
	EXT_INI_FUNCS(TechnoTypeClass);

	static void ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI);

};

#endif

#ifndef WEAPONTYPE_EXT_H
#define WEAPONTYPE_EXT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

#include <hash_map>

class WeaponTypeClassExt
{
	public:
	struct WeaponTypeClassData
	{
		// Generic
		bool Is_Initialized;
		bool Weapon_Loaded;
		WeaponTypeClass *Weapon_Source;

		// Coloured Rad Beams
		ColorStruct Beam_Color;
		bool   Beam_IsHouseColor;
		int    Beam_Duration;
		double Beam_Amplitude;
		// TS Lasers
		bool   Wave_IsHouseColor;
		bool   Wave_IsLaser;
		bool   Wave_IsBigLaser;
		ColorStruct Wave_Color;
		bool   Wave_Reverse[5];
/*
		int    Wave_InitialIntensity;
		int    Wave_IntensityStep;
		int    Wave_FinalIntensity;
*/
		// custom Ivan Bombs
		bool Ivan_KillsBridges;
		bool Ivan_Detachable;
		int Ivan_Damage;
		int Ivan_Delay;
		int Ivan_TickingSound;
		int Ivan_AttachSound;
		WarheadTypeClass *Ivan_WH;
		SHPStruct *Ivan_Image;
		int Ivan_FlickerRate;

		void Initialize(WeaponTypeClass* pThis);

	};

	EXT_P_DECLARE(WeaponTypeClass);
	EXT_FUNCS(WeaponTypeClass);
	EXT_INI_FUNCS(WeaponTypeClass);

#define idxVehicle 0
#define idxAircraft 1
#define idxBuilding 2
#define idxInfantry 3
#define idxOther 4

	static char AbsIDtoIdx(eAbstractType absId)
		{
			switch(absId)
			{
				case abs_Unit:
					return idxVehicle;
				case abs_Aircraft:
					return idxAircraft;
				case abs_Building:
					return idxBuilding;
				case abs_Infantry:
					return idxInfantry;
				default:
					return idxOther;
			}
		};

	static stdext::hash_map<BombClass *, WeaponTypeClassData *> BombExt;
	static stdext::hash_map<WaveClass *, WeaponTypeClassData *> WaveExt;

	static void ModifyWaveColor(WORD *src, WORD *dst, int Intensity, WaveClass *Wave);
};

#endif

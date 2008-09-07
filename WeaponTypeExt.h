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
		bool Weapon_Loaded;

		// Coloured Rad Beams
		bool   Beam_IsCustom;
		ColorStruct Beam_Color;
		int    Beam_Duration;
		double Beam_Amplitude;
		// TS Lasers
		bool   Wave_IsCustom;
		bool   Wave_IsLaser;
		bool   Wave_IsBigLaser;
		ColorStruct Wave_Color;
		int    Wave_InitialIntensity;
		int    Wave_IntensityStep;
		int    Wave_FinalIntensity;

		// custom Ivan Bombs
		bool Ivan_IsCustom;
		bool Ivan_KillsBridges;
		bool Ivan_Detachable;
		int Ivan_Damage;
		int Ivan_Delay;
		int Ivan_TickingSound;
		int Ivan_AttachSound;
		WarheadTypeClass *Ivan_WH;
		SHPStruct *Ivan_Image;
		int Ivan_FlickerRate;
	};

	EXT_P_DECLARE(WeaponTypeClass);
	EXT_FUNCS(WeaponTypeClass);
	EXT_INI_FUNCS(WeaponTypeClass);

	static stdext::hash_map<BombClass *, WeaponTypeClassData *> BombExt;
	static stdext::hash_map<WaveClass *, WeaponTypeClassData *> WaveExt;
	static void ModifyBeamColor(WORD *src, WORD *dst, WaveClass *Wave);
};

#endif

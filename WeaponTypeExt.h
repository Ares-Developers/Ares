#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class WeaponTypeClassExt
{
	public:
	struct WeaponTypeClassData
	{
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

	EXT_P_DEFINE(WeaponTypeClass);

	static stdext::hash_map<BombClass *, WeaponTypeClassData *> BombExt;
	static stdext::hash_map<WaveClass *, WeaponTypeClassData *> WaveExt;
	static void ModifyBeamColor(WORD *src, WORD *dst, WaveClass *Wave);
};
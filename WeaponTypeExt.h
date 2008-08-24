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
		bool   Beam_IsLaser;
		bool   Beam_IsBigLaser;

		// custom Ivan Bombs
		bool Ivan_IsCustom;
		bool Ivan_KillsBridges;
		int Ivan_Damage;
		int Ivan_Duration;
		int Ivan_TickingSound;
		int Ivan_AttachSound;
		WarheadTypeClass *Ivan_WH;
	};

	EXT_P_DEFINE(WeaponTypeClass);

	static stdext::hash_map<BombClass *, WeaponTypeClassData *> BombExt;
};
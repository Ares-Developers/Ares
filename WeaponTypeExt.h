#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class WeaponTypeClassExt
{
	public:
	struct WeaponTypeClassData
	{
		// Coloured Rad Beams
		int    BeamPeriod;
		bool   IsCustom;
		ColorStruct BeamColor;
		double BeamAmplitude;
	};

	EXT_P_DEFINE(WeaponTypeClass);
};
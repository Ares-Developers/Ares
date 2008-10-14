#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <hash_map>
#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <AnimClass.h>
#include <BulletClass.h>
#include <CaptureManagerClass.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <EMPulseClass.h>
#include <MapClass.h>
#include <TechnoClass.h>
#include <WarheadTypeClass.h>

class WarheadTypeClassExt
{
	public:
	struct WarheadTypeClassData
	{
		bool Is_Custom; // set to 1 when any feature is actually used
		bool Is_Initialized; // autoset to 1 when done initializing default values, don't touch

		bool MindControl_Permanent;

		int EMP_Duration;

		int IC_Duration;

		DynamicVectorClass<double> Verses;

		// if you want to default properties to some global values like [General]IronCurtainDuration= , do so in this
		void Initialize(WarheadTypeClass*);
	};

	EXT_P_DECLARE(WarheadTypeClass);
	EXT_FUNCS(WarheadTypeClass);
	EXT_INI_FUNCS(WarheadTypeClass);
};

#endif

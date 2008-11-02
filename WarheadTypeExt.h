#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <hash_map>
#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <BulletClass.h>
#include <CaptureManagerClass.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <EMPulseClass.h>
#include <MapClass.h>
#include <TechnoClass.h>
#include <WarheadTypeClass.h>

#include <Conversions.h>

#ifdef DEBUGBUILD
#include "Debug.h"
#endif

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

		AnimTypeClass *Temporal_WarpAway;

		// if you want to default properties to some global values like [General]IronCurtainDuration= , do so in this
		void Initialize(WarheadTypeClass*);
	};

	// evil hack
	static WarheadTypeClass *Temporal_WH;

	EXT_P_DECLARE(WarheadTypeClass);
	EXT_FUNCS(WarheadTypeClass);
	EXT_INI_FUNCS(WarheadTypeClass);
};

#endif

#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <AnimClass.h>
#include <IonBlastClass.h>
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

#include "..\_Container.hpp"

#ifdef DEBUGBUILD
#include "..\..\Misc\Debug.h"
#endif

//class WarheadTypeExt;
class WarheadTypeExt //: public Container<WarheadTypeExt>
{
public:
	typedef WarheadTypeClass TT;

	class ExtData : public Extension<TT> 
	{
	public:
		bool Is_Custom; // set to 1 when any feature is actually used

		bool MindControl_Permanent;

		int Ripple_Radius;

		int EMP_Duration;

		int IC_Duration;

		DynamicVectorClass<double> Verses;
		double DeployedDamage;

		AnimTypeClass *Temporal_WarpAway;

		ExtData(const DWORD Canary = 0) : 
			Is_Custom (false),
			MindControl_Permanent (false),
			Ripple_Radius (0),
			EMP_Duration (0),
			IC_Duration (0),
			Temporal_WarpAway (NULL),
			DeployedDamage (1.00)
			{
				for(int i = 0; i < 11; ++i) {
					Verses.AddItem(1.00);
				}
			};

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINI(TT *pThis, CCINIClass *pINI);
		virtual void InitializeRuled(TT *pThis);
	};

	static Container<WarheadTypeExt> ExtMap;

	// evil hack
	static WarheadTypeClass *Temporal_WH;

	static stdext::hash_map<IonBlastClass *, WarheadTypeExt::ExtData *> IonExt;
/*
	EXT_P_DECLARE(WarheadTypeClass);
	EXT_FUNCS(WarheadTypeClass);
	EXT_INI_FUNCS(WarheadTypeClass);
*/
};

typedef stdext::hash_map<IonBlastClass *, WarheadTypeExt::ExtData *> hash_ionExt; 
#endif

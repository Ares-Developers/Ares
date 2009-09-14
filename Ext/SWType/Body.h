#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <Helpers\Macro.h>
#include <AircraftTypeClass.h>
#include <AnimClass.h>
#include <CCINIClass.h>
#include <HouseClass.h>
#include <MissionClass.h>
#include <RadarEventClass.h>
#include <SuperClass.h>
#include <SwizzleManagerClass.h>
#include <VocClass.h>
#include <VoxClass.h>

#include "..\..\Misc\Actions.h"

#ifdef DEBUGBUILD
#include "..\..\Misc\Debug.h"
#endif

// actions for custom sw
#define SW_YES_CURSOR 0x7F
#define SW_NO_CURSOR 0x7E

// the index of the first custom sw type
#define FIRST_SW_TYPE 12

#include "..\_Container.hpp"

class SWTypeExt
{
public:
	typedef SuperWeaponTypeClass TT;

	class ExtData : public Extension<TT> 
	{
	public:
		// SpyPlane
		int SpyPlane_TypeIndex;
		int SpyPlane_Count;
		int SpyPlane_Mission;

		// Nuke
		int Nuke_Siren;

		// Sonar
		int Sonar_Range;
		int Sonar_Delay;

		// Money
		int Money_Amount;

		// Generic
		int EVA_Ready;
		int EVA_Activated;
		int EVA_Detected;

		// anim/sound
		int SW_Sound;
		AnimTypeClass *SW_Anim;
		int SW_AnimHeight;

		bool SW_TypeCustom;
		bool SW_AutoFire;
		bool SW_FireToShroud;
		bool SW_RadarEvent;
		MouseCursor SW_Cursor;
		MouseCursor SW_NoCursor;

		ExtData(const DWORD Canary = 0) : 
			SpyPlane_TypeIndex (0),
			SpyPlane_Count (0),
			SpyPlane_Mission (0),
			Nuke_Siren (-1),
			Sonar_Range (0),
			Sonar_Delay (0),
			Money_Amount (0),
			EVA_Ready (-1),
			EVA_Activated (-1),
			EVA_Detected (-1),
			SW_Sound (-1),
			SW_Anim (NULL),
			SW_AnimHeight (0),
			SW_TypeCustom (false),
			SW_AutoFire (false),
			SW_FireToShroud (true),
			SW_RadarEvent (false)
			{ };

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINI(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void InitializeRuled(TT *pThis);
	};

	static Container<SWTypeExt> ExtMap;

	static SuperWeaponTypeClass *CurrentSWType;

	bool static _stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
};

#endif

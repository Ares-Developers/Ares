#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

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
#include "..\..\Helpers\Template.h"

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
		ValueableIdx<int, AircraftTypeClass> SpyPlane_TypeIndex;
		Valueable<int> SpyPlane_Count;
		ValueableIdx<int, MissionClass> SpyPlane_Mission;

		// Nuke
		ValueableIdx<int, VocClass> Nuke_Siren;

		// Sonar
		Valueable<int> Sonar_Range;
		Valueable<int> Sonar_Delay;

		// Money
		Valueable<int> Money_Amount;

		// Generic
		ValueableIdx<int, VoxClass> EVA_Ready;
		ValueableIdx<int, VoxClass> EVA_Activated;
		ValueableIdx<int, VoxClass> EVA_Detected;

		// anim/sound
		ValueableIdx<int, VocClass> SW_Sound;
		Valueable<AnimTypeClass *> SW_Anim;
		Valueable<int> SW_AnimHeight;

		Valueable<bool> SW_TypeCustom;
		Valueable<bool> SW_AutoFire;
		Valueable<bool> SW_FireToShroud;
		Valueable<bool> SW_RadarEvent;
		Valueable<MouseCursor> SW_Cursor;
		Valueable<MouseCursor> SW_NoCursor;

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
			SpyPlane_TypeIndex (0),
			SpyPlane_Count (1),
			SpyPlane_Mission (mission_AttackAgain),
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

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void InitializeRuled(TT *pThis);
	};

	static Container<SWTypeExt> ExtMap;

	static SuperWeaponTypeClass *CurrentSWType;

	bool static _stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
};

#endif

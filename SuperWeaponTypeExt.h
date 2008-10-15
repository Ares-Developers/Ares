#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <hash_map>

#include "Actions.h"

#include <AircraftTypeClass.h>
#include <AnimClass.h>
#include <CCINIClass.h>
#include <HouseClass.h>
#include <MissionClass.h>
#include <SuperClass.h>
#include <SwizzleManagerClass.h>
#include <VocClass.h>
#include <VoxClass.h>

#ifdef DEBUGBUILD
#include "Debug.h"
#endif

// actions for custom sw
#define SW_YES_CURSOR 0x7F
#define SW_NO_CURSOR 0x7E

// the index of the first custom sw type
#define FIRST_SW_TYPE 12

class SuperWeaponTypeClassExt
{
	public:

	static SuperWeaponTypeClass *CurrentSWType;

	struct SuperWeaponTypeClassData
	{
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

		bool SW_Initialized; // if !set then still need to initialize default values - can't preinit in ctor, too early
		bool SW_TypeCustom;
		bool SW_AutoFire;
		bool SW_FireToShroud;
		MouseCursor SW_Cursor;
		MouseCursor SW_NoCursor;

		void Initialize();
	};

	EXT_P_DECLARE(SuperWeaponTypeClass);
	EXT_FUNCS(SuperWeaponTypeClass);
	EXT_INI_FUNCS(SuperWeaponTypeClass);

	bool static _stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords);

};

// New SW Type framework. See SWTypes/*.h for examples of implemented ones. Don't touch yet, still WIP.
class NewSWType
{
	protected:
		int TypeIndex;
		bool Registered;

		void Register()
			{ Array.AddItem(this); this->TypeIndex = Array.get_Count(); }

	public:
		NewSWType()
			{ Registered = 0; Register(); };

		virtual ~NewSWType()
			{ };

		static void Init();

		virtual bool CanFireAt(CellStruct* pCoords)
			{ return 1; }
		virtual bool Launch(SuperClass* pSW, CellStruct* pCoords) = 0;

		virtual void LoadFromINI(
			SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, 
			SuperWeaponTypeClass *pSW, CCINIClass *pINI) = 0;

		virtual const char * GetTypeString()
			{ return ""; }
		virtual const int GetTypeIndex()
			{ return TypeIndex; }

	static DynamicVectorClass<NewSWType *> Array;
	static NewSWType * GetNthItem(int i)
		{ return Array.GetItem(i - FIRST_SW_TYPE); }
	static int FindIndex(const char *Type)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
		{
			if(!strcmp(Array.GetItem(i)->GetTypeString(), Type))
			{
				return FIRST_SW_TYPE + i;
			}
		}
		return -1;
	}
};

#endif

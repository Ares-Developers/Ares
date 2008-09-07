#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

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

		// Animation
		AnimTypeClass *Anim_Type;
		int Anim_ExtraZ;

		// Sonar
		int Sonar_Range;
		AnimTypeClass *Sonar_Anim;
		int Sonar_Sound;
		int Sonar_Delay;

		// Generic
		int EVA_Ready;
		int EVA_Activated;
		int EVA_Detected;

		bool SW_Initialized; // if !set then still need to initialize default values - can't preinit in ctor, too early
		bool SW_TypeCustom;
		bool SW_FireToShroud;
		MouseCursor SW_Cursor;
		MouseCursor SW_NoCursor;

		void Initialize();
	};

	EXT_P_DEFINE(SuperWeaponTypeClass);
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

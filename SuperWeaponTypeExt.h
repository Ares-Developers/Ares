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

// list of new sw types
// ATTENTION: make sure the array is filled in the same order (Ext::Create)
#define SW_ANIMATION 12
#define SW_SONARPULSE 13

class SuperWeaponTypeClassExt
{
	public:

	static DynamicVectorClass<const char *> CustomSWTypes;
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
		bool SW_FireToShroud;
		MouseCursor SW_Cursor;
		MouseCursor SW_NoCursor;

		void Initialize();
	};

	EXT_P_DEFINE(SuperWeaponTypeClass);

	bool static _stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords);
	
	bool static SuperClass_Launch_Animation(SuperClass* pThis, CellStruct* pCoords);
	bool static SuperClass_Launch_SonarPulse(SuperClass* pThis, CellStruct* pCoords);

	bool static CanFireAt(SuperWeaponTypeClass *pThis, CellStruct* pCoords);

};

#endif

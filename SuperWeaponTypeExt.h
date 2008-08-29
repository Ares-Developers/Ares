#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

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

	struct SuperWeaponTypeClassData
	{
		// SpyPlane
		int SpyPlane_TypeIndex;
		int SpyPlane_Count;
		int SpyPlane_Mission;
		
		// Nuke
		int Nuke_Sound;
		
		// Generic
		int EVA_Ready;
		int EVA_Activated;
		int EVA_Detected;
		
		// Animation
		AnimTypeClass *Anim_Type;
		int Anim_ExtraZ;
		
		// Sonar
		int Sonar_Range;
		AnimTypeClass *Sonar_Anim;
		int Sonar_Sound;
	};

	EXT_P_DEFINE(SuperWeaponTypeClass);

	bool static _stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords);
	
	bool static SuperClass_Launch_Animation(SuperClass* pThis, CellStruct* pCoords);
	bool static SuperClass_Launch_SonarPulse(SuperClass* pThis, CellStruct* pCoords);

};

#endif

#ifndef GEN_PREREQ_H
#define GEN_PREREQ_H

#include <Helpers\Macro.h>
#include <CCINIClass.h>
#include <HouseClass.h>
#include <UnitTypeClass.h>

#include "_Enumerator.hpp"

#ifdef DEBUGBUILD
#include "..\Misc\Debug.h"
#endif

class HouseClass;

class GenericPrerequisite;

class GenericPrerequisite : public Enumerable<GenericPrerequisite>
{
public:
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass *pINI);

	GenericPrerequisite(const char *Title)
	{
		strncpy(this->Name, Title, 32);
		Array.AddItem(this);
	}

	virtual ~GenericPrerequisite()
	{
		Array.RemoveItem(Array.FindItemIndex(this));
	}

	DynamicVectorClass<int> Prereqs;
};

class Prereqs
{
public:
	static void Parse(char* buffer, DynamicVectorClass<int> *vec);

	static bool HouseOwnsGeneric(HouseClass *pHouse, signed int Index);
	static bool HouseOwnsBuilding(HouseClass *pHouse, signed int Index);
	static bool HouseOwnsPrereq(HouseClass *pHouse, signed int Index);

	static bool HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<int> *list);
	static bool HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<int> *list);
};

#endif

#ifndef GEN_PREREQ_H
#define GEN_PREREQ_H

#include <CCINIClass.h>
#include <HouseClass.h>
#include <UnitTypeClass.h>

#include "_Enumerator.hpp"
#include "../Ares.CRT.h"

#ifdef DEBUGBUILD
#include "../Misc/Debug.h"
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
		AresCRT::strCopy(this->Name, Title);
		Array.AddItem(this);
	}

	virtual ~GenericPrerequisite()
	{
		GenericPrerequisite *placeholder = this;
		Array.RemoveItem(Array.FindItemIndex(placeholder));
	}

	DynamicVectorClass<int> Prereqs;
};

class Prereqs
{
public:
	static void Parse(CCINIClass *pINI, const char* section, const char *key, DynamicVectorClass<int> *vec);

	static bool HouseOwnsGeneric(HouseClass *pHouse, signed int Index);
	static bool HouseOwnsSpecific(HouseClass *pHouse, signed int Index);
	static bool HouseOwnsPrereq(HouseClass *pHouse, signed int Index);

	static bool HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<int> *list);
	static bool HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<int> *list);


	typedef DynamicVectorClass<BuildingTypeClass *> BTypeList;

	static bool ListContainsGeneric(BTypeList * List, signed int Index);
	static bool ListContainsSpecific(BTypeList * List, signed int Index);
	static bool ListContainsPrereq(BTypeList * List, signed int Index);

	static bool ListContainsAll(BTypeList * List, DynamicVectorClass<int> *Requirements);
	static bool ListContainsAny(BTypeList * List, DynamicVectorClass<int> *Requirements);

};

#endif

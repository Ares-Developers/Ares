#ifndef GEN_PREREQ_H
#define GEN_PREREQ_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <CCINIClass.h>
#include <HouseClass.h>
#include <UnitTypeClass.h>

class HouseClass;

class GenericPrerequisite
{
public:
	static DynamicVectorClass< GenericPrerequisite* > Array;

	GenericPrerequisite(const char *Title)
	{
		strncpy(this->Name, Title, 32);
		Array.AddItem(this);
	}

	static int FindIndex(const char *Title)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
			if(!strcmp(Title, Array.GetItem(i)->Name))
				return i;
		return -1;
	}

	static GenericPrerequisite* Find(const char *Title)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
			if(!strcmp(Title, Array.GetItem(i)->Name))
				return Array.GetItem(i);
		return NULL;
	}

	static GenericPrerequisite* FindOrAllocate(const char *Title)
	{
		GenericPrerequisite *find = Find(Title);
		return find ? find : new GenericPrerequisite(Title);
	}

	static void ClearArray()
	{
		while(int len = Array.get_Count())
		{
			delete Array[len];
			Array.RemoveItem(len);
		}
	}

	static void LoadFromINIList(CCINIClass *pINI);
	void LoadFromINI(CCINIClass *pINI);

	char Name[32];
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

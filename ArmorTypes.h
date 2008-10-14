#ifndef ARMORS_H
#define ARMORS_H

#include <ArrayClasses.h>
#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include "WarheadTypeExt.h"

class ArmorType
{
	public:
		char Title[32];
		int DefaultIndex;

	ArmorType(const char *Name)
	{
		strncpy(this->Title, Name, 31);
		DefaultIndex = -1;
		Array.AddItem(this);
	}

	static DynamicVectorClass<ArmorType*> Array;

	static ArmorType *FindOrAllocate(const char *Name)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
		{
			if(!_strcmpi(Array[i]->Title, Name))
			{
				return Array[i];
			}
		}
		return new ArmorType(Name);
	}

	static int FindIndex(const char *Name)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
		{
			if(!_strcmpi(Array[i]->Title, Name))
			{
				return i;
			}
		}
		return -1;
	}

	static void LoadFromINIList(CCINIClass *pINI);
	static void LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH);
};

#endif

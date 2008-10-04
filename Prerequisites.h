#ifndef GEN_PREREQ_H
#define GEN_PREREQ_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

class GenericPrerequisite
{
public:
	static DynamicVectorClass< GenericPrerequisite* > Array;

	GenericPrerequisite(const char *Title)
	{
		this->Name = _strdup(Title);
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

	static void LoadFromINIList(CCINIClass *pINI);
	void LoadFromINI(CCINIClass *pINI);


	const char * Name;
	DynamicVectorClass<int> Prereqs;

};

class Prereqs
{
public:
	static void Parse(char* buffer, DynamicVectorClass<int> *vec);
};

#endif

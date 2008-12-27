#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <ArrayClasses.h>
#include <CCINIClass.h>

template <typename T> class Enumerable
{
public:
	static DynamicVectorClass< T* > Array;

	static int FindIndex(const char *Title)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
			if(!_strcmpi(Title, Array.GetItem(i)->Name))
				return i;
		return -1;
	}

	static T* Find(const char *Title)
	{
		for(int i = 0; i < Array.get_Count(); ++i)
			if(!_strcmpi(Title, Array.GetItem(i)->Name))
				return Array.GetItem(i);
		return NULL;
	}

	static T* FindOrAllocate(const char *Title)
	{
		T *find = Find(Title);
		return find ? find : new T(Title);
	}

	static void ClearArray()
	{
		while(int len = Array.get_Count())
		{
			delete Array[len];
			Array.RemoveItem(len);
		}
	}

	static void LoadFromINIList(CCINIClass *pINI)
	{
		const char *section = GetMainSection();
//		char section[] = "GenericPrerequisites";
		int len = pINI->GetKeyCount(section);
		for(int i = 0; i < len; ++i)
		{
			const char *Key = pINI->GetKeyName(section, i);
			FindOrAllocate(Key);
		}
		for(int i = 0; i < Array.get_Count(); ++i)
		{
			Array[i]->LoadFromINI(pINI);
		}
	}

	Enumerable()
	{
		;
	}

	virtual ~Enumerable()
	{
		;
	}

//	template <typename T2>
	static const char * GetMainSection();

	char Name[32];
};

#endif

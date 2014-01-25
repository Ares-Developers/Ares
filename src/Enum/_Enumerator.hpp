#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <algorithm>
#include <functional>

#include <ArrayClasses.h>
#include <CCINIClass.h>

template <typename T> class Enumerable
{
public:
	static DynamicVectorClass< T* > Array;

	static int FindIndex(const char *Title)
	{
		auto result = std::find_if(Array.begin(), Array.end(), [Title](Enumerable<T>* Item) {
			return !_strcmpi(Item->Name, Title);
		});
		if(result == Array.end()) {
			return -1;
		}
		return std::distance(Array.begin(), result);
	}

	static T* Find(const char *Title)
	{
		auto result = FindIndex(Title);
		return (result < 0) ? nullptr : Array[result];
	}

	static T* FindOrAllocate(const char *Title)
	{
		T *find = Find(Title);
		return find ? find : new T(Title);
	}

	static void ClearArray()
	{
		for(int i = Array.Count - 1; i >= 0; --i) {
			delete Array[i];
		}
		Array.Clear();
	}

	static void LoadFromINIList(CCINIClass *pINI)
	{
		const char *section = GetMainSection();
		int len = pINI->GetKeyCount(section);
		for(int i = 0; i < len; ++i) {
			const char *Key = pINI->GetKeyName(section, i);
			FindOrAllocate(Key);
		}
		for(int i = 0; i < Array.Count; ++i) {
			Array[i]->LoadFromINI(pINI);
		}
	}

	Enumerable()
	{
		Array.AddItem(static_cast<T*>(this));
	}

	Enumerable(const char* Title) {
		this->Name[0] = 0;

		if(Title) {
			AresCRT::strCopy(this->Name, Title);
		}

		Array.AddItem(static_cast<T*>(this));
	}

	virtual ~Enumerable()
	{
		Array.RemoveItem(Array.FindItemIndex(static_cast<T*>(this)));
	}

//	template <typename T2>
	static const char * GetMainSection();

	char Name[32];
};

#endif

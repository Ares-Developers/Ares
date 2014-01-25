#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <algorithm>
#include <functional>

#include <ArrayClasses.h>
#include <CCINIClass.h>

template <typename T> class Enumerable
{
	typedef std::vector<T*> container_t;
public:
	static container_t Array;

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
		for(auto i = Array.size(); i > 0; --i) {
			delete Array[i - 1];
		}
		Array.clear();
	}

	static void LoadFromINIList(CCINIClass *pINI)
	{
		const char *section = GetMainSection();
		int len = pINI->GetKeyCount(section);
		for(int i = 0; i < len; ++i) {
			const char *Key = pINI->GetKeyName(section, i);
			FindOrAllocate(Key);
		}
		for(size_t i = 0; i < Array.size(); ++i) {
			Array[i]->LoadFromINI(pINI);
		}
	}

	Enumerable()
	{
		Array.push_back(static_cast<T*>(this));
	}

	Enumerable(const char* Title) {
		this->Name[0] = 0;

		if(Title) {
			AresCRT::strCopy(this->Name, Title);
		}

		Array.push_back(static_cast<T*>(this));
	}

	virtual ~Enumerable()
	{
		Array.erase(std::remove(Array.begin(), Array.end(), static_cast<T*>(this)), Array.end());
	}

	static const char * GetMainSection();

	char Name[32];
};

#endif

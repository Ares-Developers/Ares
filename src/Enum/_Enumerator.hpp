#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <algorithm>
#include <functional>
#include <memory>

#include <ArrayClasses.h>
#include <CCINIClass.h>

template <typename T> class Enumerable
{
	typedef std::vector<std::unique_ptr<T>> container_t;
public:
	static container_t Array;

	static int FindIndex(const char *Title)
	{
		auto result = std::find_if(Array.begin(), Array.end(), [Title](std::unique_ptr<T> &Item) {
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
		return (result < 0) ? nullptr : Array[result].get();
	}

	static T* FindOrAllocate(const char *Title)
	{
		if(T *find = Find(Title)) {
			return find;
		}
		Array.push_back(std::make_unique<T>(Title));
		return Array.back().get();
	}

	static void ClearArray()
	{
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
	}

	Enumerable(const char* Title) {
		this->Name[0] = 0;

		if(Title) {
			AresCRT::strCopy(this->Name, Title);
		}
	}

	virtual ~Enumerable()
	{
	}

	static const char * GetMainSection();

	char Name[32];
};

#endif

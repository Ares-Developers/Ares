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

	struct comparator : public std::binary_function<Enumerable<T>*, const char *, bool> {
		bool operator()(Enumerable<T>* Item, const char* title) const { return !_strcmpi(Item->Name, title); }
	};

	static T** stl_Find(const char *Title) {
		return std::find_if(Array.start(), Array.end(), std::bind2nd(comparator(), Title));
	}

	static int FindIndex(const char *Title)
	{
		for(int i = 0; i < Array.Count; ++i)
			if(!_strcmpi(Title, Array.GetItem(i)->Name))
				return i;
		return -1;
	}

	static T* Find(const char *Title)
	{
/*		for(int i = 0; i < Array.get_Count(); ++i)
			if(!_strcmpi(Title, Array.GetItem(i)->Name))
				return Array.GetItem(i);
*/
		T** result = Enumerable<T>::stl_Find(Title);
		if(result == Array.end()) {
			return NULL;
		}
		return *result;
	}

	static T* FindOrAllocate(const char *Title)
	{
		T *find = Find(Title);
		return find ? find : new T(Title);
	}

	static void ClearArray()
	{
		while(int len = Array.Count) {
			delete Array[len];
			Array.RemoveItem(len);
		}
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

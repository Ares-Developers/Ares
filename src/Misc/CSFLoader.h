//This enables the loading of additional CSF files that
//- add labels to the current stringtable
//- overwrite existing labels

//code translated from RP2 and optimized :3

#ifndef CSFLOADER_H
#define CSFLOADER_H

#include <CCFileClass.h>
#include <StringTable.h>

#define CSF_MAX_ENTRIES 20000

class CSFLoader
{
public:
	static int CSFCount;
	static int NextValueIndex;

	static void LoadAdditionalCSF(const char* fileName);

	static std::hash_map<std::string, const CSFString*> DynamicStrings;

	static const CSFString* FindDynamic(const char* name);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def);
};

#endif

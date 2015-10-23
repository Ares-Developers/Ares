//This enables the loading of additional CSF files that
//- add labels to the current stringtable
//- overwrite existing labels

//code translated from RP2 and optimized :3

#pragma once

#include <CCFileClass.h>
#include <StringTable.h>

#include <unordered_map>

class CSFLoader
{
public:
	static auto const MaxEntries = 20000u;

	static int CSFCount;
	static int NextValueIndex;

	static void LoadAdditionalCSF(const char* fileName, bool ignoreLanguage = false);

	static std::unordered_map<std::string, const CSFString*> DynamicStrings;

	static const CSFString* FindDynamic(const char* name);
	static const wchar_t* GetDynamicString(const char* name, const wchar_t* pattern, const char* def);
};

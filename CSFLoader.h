//This enables the loading of additional CSF files that
//- add labels to the current stringtable
//- overwrite existing labels

//code translated from RP2 and optimized :3

#ifndef CSFLOADER_H
#define CSFLOADER_H

#include <CCFileClass.h>
#include <StringTable.h>

#define CSF_MAX_ENTRIES 8000

class CSFLoader
{
public:
	static int CSFCount;
	static int NextValueIndex;

	static void LoadAdditionalCSF(const char* fileName);
};

#endif

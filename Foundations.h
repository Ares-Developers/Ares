#ifndef FOUNDATIONS_H
#define FOUNDATIONS_H

#include "Ares.h"
#include HASHMAP

#define FOUNDATION_CUSTOM	0x7F

class Foundations
{
public:
	struct CustomFoundationStruct
	{
		bool IsCustom;
		int CustomWidth;
		int CustomHeight;
		CellStruct* CustomData;
	};

	static stdext::hash_map<BuildingTypeClass*,CustomFoundationStruct> CustomFoundation;

	static void __stdcall Defaults(BuildingTypeClass*);
	static void __stdcall Load(BuildingTypeClass*,IStream*);
	static void __stdcall Save(BuildingTypeClass*,IStream*);
	static void __stdcall LoadFromINI(BuildingTypeClass*,CCINIClass*);
};

#endif

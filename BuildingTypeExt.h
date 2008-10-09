#ifndef BUILDINGTYPE_EXT_H
#define BUILDINGTYPE_EXT_H

#include "Ares.h"
#include <hash_map>

#define FOUNDATION_CUSTOM	0x7F

class BuildingTypeClassExt
{
public:
	struct BuildingTypeClassData
	{
		// foundations
		bool IsCustom;
		int CustomWidth;
		int CustomHeight;
		CellStruct* CustomData;

		// new secret lab
		DynamicVectorClass<TechnoTypeClass *> Secret_Boons;
		bool Secret_RecalcOnCapture;

		// these two are to initialize things that can't be inited in ctor (e.g. things that default to rules values)
		bool Data_Initialized;
		void Initialize(BuildingTypeClass *pThis);
	};

	static stdext::hash_map<BuildingTypeClass*, BuildingTypeClassData> Ext_v;

	static void __stdcall Defaults(BuildingTypeClass*);
	static void __stdcall Load(BuildingTypeClass*,IStream*);
	static void __stdcall Save(BuildingTypeClass*,IStream*);
	static void __stdcall LoadFromINI(BuildingTypeClass*,CCINIClass*);
};

#endif
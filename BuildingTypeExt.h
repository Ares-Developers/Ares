#ifndef BUILDINGTYPE_EXT_H
#define BUILDINGTYPE_EXT_H

#define BTEXT_VALIDATION 0xAFFEAFFE //double monkey!

#include <hash_map>

#include <CCINIClass.h>
#include <BuildingTypeClass.h>
#include <InfantryTypeClass.h>
#include <Randomizer.h>
#include <UnitTypeClass.h>

#include "Ares.h"

//ifdef DEBUGBUILD -- legit needs to log things, so no debug
#include "Debug.h"
//endif

class BuildingClass;

#define FOUNDATION_CUSTOM	0x7F

class BuildingTypeClassExt
{
public:
	class Struct
	{
	public:
		// validation value for savegames
		// not involved in any functionality, but I'm using this to find savegame problems, so keep it! -pd
		DWORD SavegameValidation;

		// foundations
		bool IsCustom;
		int CustomWidth;
		int CustomHeight;
		CellStruct* CustomData;

		// new secret lab
		DynamicVectorClass<TechnoTypeClass *> Secret_Boons;
		bool Secret_RecalcOnCapture;
		bool Secret_Placed;

		bool IsInitialized;
		void Initialize(BuildingTypeClass *pThis);

		Struct()
		{
			SavegameValidation = BTEXT_VALIDATION;
			IsInitialized = false;
			IsCustom = false;
			CustomData = NULL;
			CustomWidth = 0;
			CustomHeight = 0;
		}
	};

	static stdext::hash_map<BuildingTypeClass*, Struct> Map;

	static void UpdateSecretLabOptions(BuildingClass *pThis);
};

#endif
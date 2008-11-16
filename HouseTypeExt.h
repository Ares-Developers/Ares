#ifndef HTEXT_H
#define HTEXT_H

#define HTEXT_VALIDATION 0x12345678

#include <CCINIClass.h>
#include <hash_map>
#include <HouseTypeClass.h>
#include <BuildingTypeClass.h>
#include <PCX.h>
#include <StringTable.h>

//TODO: Rename to HouseTypeExt
//TODO: Load/Save from savegames

class HouseTypeExt
{
public:
	class Struct
	{
	public:
		// validation value for savegames
		// not involved in any functionality, but I'm using this to find savegame problems, so keep it! -pd
		DWORD SavegameValidation;
		char FlagFile[0x20]; //Flag
		char LSFile[0x20]; //LoadScreen
		char LSPALFile[0x20]; //LoadScreen palette
		char TauntFile[0x20]; //Taunt filename format (should contain %d !!!)
		char LSName[0x20]; //Stringtable label
		char LSSpecialName[0x20]; //Stringtable label for this country's special weapon
		char LSBrief[0x20]; //Stringtable label for this country's load brief
		char StatusText[0x20]; //Stringtable label for this country's Skirmish STT
		int RandomSelectionWeight; //This country gets added this many times into the list of legible countries for random selection.

		DynamicVectorClass<BuildingTypeClass *> Powerplants;

		bool Data_Initialized;

		void Initialize(HouseTypeClass *pThis);
	};

	static stdext::hash_map<HouseTypeClass*, Struct> Map;
	static int PickRandomCountry();
};

#endif

#ifndef COUNTRIES_H
#define COUNTRIES_H

#include "Ares.h"
#include HASHMAP

//TODO: Load/Save from savegames

class Countries
{
public:
	struct CountryExtensionStruct
	{
		char	FlagFile[0x20];			//Flag
		char	LSFile[0x20];			//LoadScreen
		char	LSPALFile[0x20];		//LoadScreen palette
		char	TauntFile[0x20];		//Taunt filename format (should contain %d !!!)
		char	LSName[0x20];			//Stringtable label
		char	LSSpecialName[0x20];	//Stringtable label for this country's special weapon
		char	LSBrief[0x20];			//Stringtable label for this country's load brief
		char	StatusText[0x20];		//Stringtable label for this country's Skirmish STT
		int		RandomSelectionWeight;	//This country gets added this many times into the list of legible countries for random selection.
	};

	static stdext::hash_map<HouseTypeClass*,CountryExtensionStruct> CountryExt;

	static void __stdcall Construct(HouseTypeClass*);
	static void __stdcall LoadFromINI(HouseTypeClass*, CCINIClass*);

	static int PickRandomCountry();
};

#endif

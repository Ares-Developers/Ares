#ifndef SIDES_H
#define SIDES_H

#define SIDEEXT_VALIDATION 0x87654321

#include "Ares.h"
#include <CCINIClass.h>

#include <hash_map>

//TODO: Load/Save from savegames
class VoxClass;

class SideExt
{
public:
	class Struct
	{
	public:
		DWORD SavegameValidation;
		InfantryTypeClass* DefaultDisguise;
		InfantryTypeClass* Crew;
		int SurvivorDivisor;
		TypeList<BuildingTypeClass*> BaseDefenses;
		TypeList<int> BaseDefenseCounts;
		BuildingTypeClass* PowerPlant;
		ColorScheme* LoadTextColor;
		TypeList<TechnoTypeClass*> ParaDrop;
		TypeList<int> ParaDropNum;
		int SidebarMixFileIndex;
		bool SidebarYuriFileNames;
		char EVATag[0x20];	//TODO

		Struct()
		{
			SavegameValidation = SIDEEXT_VALIDATION;
			DefaultDisguise = NULL;
			Crew = NULL;
			PowerPlant = NULL;
			LoadTextColor = NULL;
			*EVATag = 0;
		}
	};

	struct VoxFileNameStruct //need to make this a struct for certain reasons
	{
		char FileName[0x10];

		bool operator == (VoxFileNameStruct t)
			{ return (_strcmpi(FileName, t.FileName) == 0); }
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static stdext::hash_map<SideClass*, Struct> Map;
	static stdext::hash_map<VoxClass*, DynamicVectorClass<VoxFileNameStruct> > EVAFiles;

	static DWORD BaseDefenses(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD Disguise(REGISTERS* R, DWORD dwReturnAddress, bool bUseESI);
	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);
};

#endif

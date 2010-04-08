#include "RMG.h"
#include "../UI/registered.h"
#include "Ares.h"

#include <HouseClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>

bool RMG::UrbanAreas = 0;
bool RMG::UrbanAreasRead = 0;

DynamicVectorClass<char*> RMG::UrbanStructures;
int RMG::UrbanStructuresReadSoFar;
DynamicVectorClass<char*> RMG::UrbanVehicles;
DynamicVectorClass<char*> RMG::UrbanInfantry;

//0x596FFE
DEFINE_HOOK(596FFE, RMG_EnableArchipelago, 0)
{
	R->EBP(0);						//start at index 0 instead of 1
	R->EBX(0x82B034);				//set the list offset to "TXT_MAP_ARCHIPELAGO"
	return 0x597008;
}

//0x5970EA
DEFINE_HOOK(5970EA, RMG_EnableDesert, 9)
{
	GET(HWND, hWnd, EDI);

	//List the item
	LRESULT result=
		SendMessageA(
		hWnd,
		WW_CB_ADDITEM,		//CUSTOM BY WESTWOOD
		0,
		(LPARAM)StringTable::LoadString("Name:Desert")); // oh pd

	//Set the item data
	SendMessageA(
		hWnd,
		CB_SETITEMDATA,
		result,
		3);			//Make it actually be desert

	return 0;
}

DEFINE_HOOK(596C81, MapSeedClass_DialogFunc_GetData, 5)
{
	GET(HWND, hDlg, EBP);
	HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS);
	if(hDlgItem) {
		RMG::UrbanAreas = (1 == SendMessageA(hDlgItem, BM_GETCHECK, 0, 0));
	}
	return 0;
}

DEFINE_HOOK(5982D5, MapSeedClass_LoadFromINI, 6)
{
	if(!RMG::UrbanAreasRead) {
		GET(CCINIClass *, pINI, EDI);
		RMG::UrbanAreas = pINI->ReadBool("General", "GenerateUrbanAreas", RMG::UrbanAreas);

		if(!pINI->GetSection("Urban")) {
			// no [Urban] section - use our defaults; ASSUMES default rules...

			RMG::UrbanStructures.Clear();
			char *defStructures [] = {"CITY01", "CITY02", "CITY03", "CITY04", "CITY05", "CITY06", "CANEWY20", "CANEWY21", 0};
			for(char ** cur = defStructures; *cur && **cur; ++cur) {
				RMG::UrbanStructures.AddItem(*cur);
			}

			RMG::UrbanVehicles.Clear();
			char *defVehicles[] = {"TRUCKA", "TRUCKB", "COP", "EUROC", "SUVW", "SUVB", "FTRK", "AMBU", 0};
			for(char ** cur = defVehicles; *cur && **cur; ++cur) {
				RMG::UrbanVehicles.AddItem(*cur);
			}

			RMG::UrbanInfantry.Clear();
			char *defInfantry[] = {"CIV1", "CIV2", "CIV3", "CIVA", "CIVB", "CIVC", 0};
			for(char ** cur = defInfantry; *cur && **cur; ++cur) {
				RMG::UrbanInfantry.AddItem(*cur);
			}
		} else {
			if(pINI->ReadString("Urban", "Structures", "", Ares::readBuffer, Ares::readLength)) {
				RMG::UrbanStructures.Clear();
				for(char * cur = strtok(Ares::readBuffer, ","); cur && *cur; cur = strtok(NULL, ",")) {
					if(BuildingTypeClass::FindIndex(cur) != -1) {
						RMG::UrbanStructures.AddItem(cur);
					}
				}
			}

			if(pINI->ReadString("Urban", "Infantry", "", Ares::readBuffer, Ares::readLength)) {
				RMG::UrbanInfantry.Clear();
				for(char * cur = strtok(Ares::readBuffer, ","); cur && *cur; cur = strtok(NULL, ",")) {
					if(InfantryTypeClass::FindIndex(cur) != -1) {
						RMG::UrbanInfantry.AddItem(cur);
					}
				}
			}

			if(pINI->ReadString("Urban", "Vehicles", "", Ares::readBuffer, Ares::readLength)) {
				RMG::UrbanVehicles.Clear();
				for(char * cur = strtok(Ares::readBuffer, ","); cur && *cur; cur = strtok(NULL, ",")) {
					if(UnitTypeClass::FindIndex(cur) != -1) {
						RMG::UrbanVehicles.AddItem(cur);
					}
				}
			}
		}

		RMG::UrbanAreasRead = 1;
	}
	return 0;
}

DEFINE_HOOK(598FB8, RMG_GenerateUrban, 5)
{
	if(RMG::UrbanAreas) {
		GET(void *, pMapSeed, ESI);
		SET_REG32(ecx, pMapSeed);
		CALL(0x5A5020);
	}
	return 0;
}

DEFINE_HOOK(5A65CA, MapSeedClass_Generate_PlaceUrbanStructures_Start, 5)
{
	RMG::UrbanStructuresReadSoFar = 0;
	if(!RMG::UrbanStructures.Count) {
		return 0x5A68F1; // no structures - nothing to do
	}
	R->ESI<char **>(RMG::UrbanStructures.Items);
	return 0x5A65D5;
}

DEFINE_HOOK(5A6619, MapSeedClass_Generate_PlaceUrbanStructures_Loop, 6)
{
	GET_STACK(int, Count, 0x78);
	++RMG::UrbanStructuresReadSoFar;
	return (RMG::UrbanStructures.Count >= RMG::UrbanStructuresReadSoFar)
		? 0x5A65D1
		: 0x5A6621
	;
}

DEFINE_HOOK(5A66A6, MapSeedClass_Generate_PlaceUrbanStructures_CTOR, 5)
{
	GET(char **, names, ECX);
	GET(DWORD, idx, EAX);

	R->EDX<DWORD>(idx);
	R->ECX<char *>(names[idx]);
	return 0x5A66AB;
}

DEFINE_HOOK(5A69B9, MapSeedClass_Generate_PlaceUrbanFoots, 5)
{
	GET(int, Index, EAX);
	int Length = RMG::UrbanInfantry.Count + RMG::UrbanVehicles.Count;
	if(Length == 0) {
		return 0x5A6B96; // no possible items - nothing to do
	}
	if(Index < 0 || Index >= Length) {
		return 0x5A6998;
	}

	GET(HouseClass *, Owner, EBP);
	ObjectClass *Item = NULL;
	if(Index < RMG::UrbanInfantry.Count) {
		InfantryTypeClass *IType = InfantryTypeClass::Find(RMG::UrbanInfantry[Index]);
		Item = IType->CreateObject(Owner);
	} else {
		Index -= RMG::UrbanInfantry.Count;
		UnitTypeClass *UType = UnitTypeClass::Find(RMG::UrbanVehicles[Index]);
		Item = UType->CreateObject(Owner);
	}
	R->ESI<ObjectClass *>(Item);

	return 0x5A6A31;
}


DEFINE_HOOK(59000E, RMG_FixPavedRoadEnd_Bridges_North, 0)
{
	return 0x590087;
}

DEFINE_HOOK(5900F7, RMG_FixPavedRoadEnd_Bridges_South, 0)
{
	return 0x59015E;
}

DEFINE_HOOK(58FCC6, RMG_FixPavedRoadEnd_Bridges_West, 0)
{
	return 0x58FD2A;
}

DEFINE_HOOK(58FBDD, RMG_FixPavedRoadEnd_Bridges_East, 0)
{
	return 0x58FC55;
}

DEFINE_HOOK(58FA51, RMG_PlaceWEBridge, 6)
{
	LEA_STACK(RectangleStruct *, pRect, 0x14);

	//it's a WE bridge
	return (pRect->Width > pRect->Height)
	 ? 0x58FA73
	 : 0;
}

DEFINE_HOOK(58FE7B, RMG_PlaceNSBridge, 8)
{
	LEA_STACK(RectangleStruct *, pRect, 0x14);

	//it's a NS bridge
	return (pRect->Height > pRect->Width)
	 ? 0x58FE91
	 : 0;
}

DEFINE_HOOK(545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 7)
{
	if(R->EAX() == -1) {
		// all theaters except snow have this set, so I'll assume that this was tripped by snow.
		// don't like it? put the damned tag in the INI.
		R->EAX(71);
	}
	return 0;
}

#include "RMG.h"
#include "../UI/registered.h"
#include "../Ares.h"

#include <HouseClass.h>
#include <OverlayTypeClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <Randomizer.h>
#include <MapSeedClass.h>

bool RMG::UrbanAreas = 0;
bool RMG::UrbanAreasRead = 0;

int RMG::UrbanStructuresReadSoFar;
VectorNames RMG::UrbanStructures;
VectorNames RMG::UrbanVehicles;
VectorNames RMG::UrbanInfantry;

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
		reinterpret_cast<LPARAM>(StringTable::LoadString("Name:Desert"))); // oh pd

	//Set the item data
	SendMessageA(
		hWnd,
		CB_SETITEMDATA,
		result,
		3);			//Make it actually be desert

	return 0;
}

// #882 select from all available options and randomize urban areas
DEFINE_HOOK(596786, MapSeedClass_DialogFunc_SurpriseMe, 9)
{
	GET(HWND, hDlg, EBP);
	Randomizer* pRand = Randomizer::Global();
	MapSeedClass* pMapSeed = MapSeedClass::Global();

	// selects map terrain type from all the items in the combobox
	if(HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, 0x405)) {
		int count = SendMessageA(hDlgItem, CB_GETCOUNT, 0, 0);
		int index = pRand->RandomRanged(0, count - 1);
		int itemdata = SendMessageA(hDlgItem, CB_GETITEMDATA, index, 0);
		pMapSeed->MapType = itemdata;
	}

	// selects theater / climate from all the items in the combobox
	if(HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, 0x407)) {
		int count = SendMessageA(hDlgItem, CB_GETCOUNT, 0, 0);
		int index = pRand->RandomRanged(0, count - 1);
		int itemdata = SendMessageA(hDlgItem, CB_GETITEMDATA, index, 0);
		pMapSeed->Theater = itemdata;
	}

	// randomize creation of urban areas
	if(HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS)) {
		int enabled = pRand->RandomRanged(1, 100);
		RMG::UrbanAreas = (enabled > 50);
	}

	// recreate random value for "map time of the day"
	R->EAX(pRand->RandomRanged(0, 3));
	return 0x5967C1;
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

DEFINE_HOOK(5971EA, MapSeedClass_DialogFunc_SetData, 5)
{
	GET(HWND, hDlg, EBX);
	HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_RMG_URBAN_AREAS);
	if(hDlgItem) {
		SendMessageA(hDlgItem, BM_SETCHECK, (RMG::UrbanAreas ? 1 : 0), 0);
	}
	return 0;
}

//speeds up preview drawing by insane amounts
DEFINE_HOOK(5FED00, OverlayTypeClass_GetRadarColor, 0)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct *, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

DEFINE_HOOK(5982D5, MapSeedClass_LoadFromINI, 6)
{
	if(!RMG::UrbanAreasRead) {
		GET(CCINIClass *, pINI, EDI);
		RMG::UrbanAreas = pINI->ReadBool("General", "GenerateUrbanAreas", RMG::UrbanAreas);

		//I can should this be theater-related in the future... ~pd

		pINI->ReadString("Urban", "Structures",
			"CABUNK01,CABUNK02,CAARMY01,CAARMY02,CAARMY03,CAARMY04,CACHIG03,CANEWY01,CANEWY14,CANWY09,CANWY26,CANWY25,CATEXS07",
			Ares::readBuffer);
		RMG::UrbanStructures.Tokenize(Ares::readBuffer);

		pINI->ReadString("Urban", "Infantry",
			"CIV1,CIV2,CIV3,CIVA,CIVB,CIVC",
			Ares::readBuffer);
		RMG::UrbanInfantry.Tokenize(Ares::readBuffer);

		pINI->ReadString("Urban", "Vehicles",
			"TRUCKA,TRUCKB,COP,EUROC,SUVW,SUVB,FTRK,AMBU"
			, Ares::readBuffer);
		RMG::UrbanVehicles.Tokenize(Ares::readBuffer);

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
	if(!RMG::UrbanStructures.Count()) {
		return 0x5A68F1; // no structures - nothing to do
	}
	R->ESI(RMG::UrbanStructures.ToString());
	return 0x5A65D5;
}

DEFINE_HOOK(5A6619, MapSeedClass_Generate_PlaceUrbanStructures_Loop, 6)
{
	++RMG::UrbanStructuresReadSoFar;
	return (RMG::UrbanStructures.Count() > RMG::UrbanStructuresReadSoFar)
		? 0x5A65D1
		: 0x5A6621
	;
}

DEFINE_HOOK(5A66B0, MapSeedClass_Generate_PlaceUrbanStructures_SanityCheck, 5)
{
	GET(int, Index, EAX);
	return (Index > -1)
		? 0
		: 0x5A68D8
	;
}

DEFINE_HOOK(5A6998, MapSeedClass_Generate_PlaceUrbanFoots, 5)
{
	int Length = RMG::UrbanInfantry.Count() + RMG::UrbanVehicles.Count();
	if(Length == 0) {
		return 0x5A6B96; // no possible items - nothing to do
	}

	int Index = Randomizer::Global()->RandomRanged(0, Length - 1);

	GET(HouseClass *, Owner, EBP);
	ObjectClass *Item = nullptr;
	if(Index < RMG::UrbanInfantry.Count()) {
		if(auto const IType = InfantryTypeClass::Find(RMG::UrbanInfantry[Index])) {
			Item = IType->CreateObject(Owner);
		} else {
			Debug::Log("Unknown InfantryType %s in RMG config!\n", RMG::UrbanInfantry[Index]);
		}
	} else {
		Index -= RMG::UrbanInfantry.Count();
		if(auto const UType = UnitTypeClass::Find(RMG::UrbanVehicles[Index])) {
			Item = UType->CreateObject(Owner);
		} else {
			Debug::Log("Unknown VehicleType %s in RMG config!\n", RMG::UrbanVehicles[Index]);
		}
	}
	R->ESI<ObjectClass *>(Item);

	return 0x5A6A31;
}

// ==============================

DEFINE_HOOK(5A5C6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNE, 9)
{
	return 0x5A5CC8;
}

DEFINE_HOOK(5A5D6F, MapSeedClass_Generate_PlacePavedRoads_RoadEndSW, 9)
{
	return 0x5A5DB8;
}

DEFINE_HOOK(5A5F6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNW, 8)
{
	return 0x5A5FF8;
}

DEFINE_HOOK(5A6464, MapSeedClass_Generate_PlacePavedRoads_RoadEndSE, 9)
{
	return 0x5A64AD;
}

// ==============================

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

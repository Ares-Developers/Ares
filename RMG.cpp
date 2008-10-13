#include "Ares.h"
#include <MacroHelpers.h> //basically indicates that this is DCoder country

//0x596FFE
EXPORT RMG_EnableArchipelago(REGISTERS* R)
{
	R->set_EBP(0);						//start at index 0 instead of 1
	R->set_EBX(0x82B034);				//set the list offset to "TXT_MAP_ARCHIPELAGO"
	return 0x597008;
}

//0x5970EA
EXPORT RMG_EnableDesert(REGISTERS* R)
{
	HWND hWnd=(HWND)R->get_EDI();

	//List the item
	LRESULT result=
		SendMessageA(
		hWnd,
		WW_CB_ADDITEM,		//CUSTOM BY WESTWOOD
		0,
		(LPARAM)StringTable::LoadString("TXT_BIOME_DESERT"));

	//Set the item data
	SendMessageA(
		hWnd,
		CB_SETITEMDATA,
		result,
		3);			//Make it actually be desert

	return 0;
}

//0x598FB8
EXPORT RMG_GenerateUrban(REGISTERS* R)
{
	DEBUGLOG("RMG: Generating urban areas\n");

	void* pMapSeed = (void*)R->get_ESI();
	SET_REG32(ecx, pMapSeed);
	CALL(0x5A5020);
}

//0x59000E
EXPORT RMG_FixPavedRoadEnd_Bridges_North(REGISTERS* R)
{ return 0x590087; }
//0x5900F7
EXPORT RMG_FixPavedRoadEnd_Bridges_South(REGISTERS* R)
{ return 0x59015E; }
//0x58FCC6
EXPORT RMG_FixPavedRoadEnd_Bridges_West(REGISTERS* R)
{ return 0x58FD2A; }
//0x58FBDD
EXPORT RMG_FixPavedRoadEnd_Bridges_East(REGISTERS* R)
{ return 0x58FC55; }

//0x58FA51
EXPORT RMG_PlaceWEBridge(REGISTERS* R)
{
	RectangleStruct* pRect = (RectangleStruct*)((BYTE*)R->get_ESP()+0x14);

	if(pRect->Width > pRect->Height)	//it's a WE bridge
		return 0x58FA73;
	else
		return 0;
}

//0x58FE7B
EXPORT RMG_PlaceNSBridge(REGISTERS* R)
{
	RectangleStruct* pRect = (RectangleStruct*)((BYTE*)R->get_ESP()+0x14);

	if(pRect->Height > pRect->Width)	//it's a NS bridge
		return 0x58FE91;
	else
		return 0;
}

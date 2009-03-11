#include <StringTable.h>

//0x596FFE
DEFINE_HOOK(596FFE, RMG_EnableArchipelago, 0)
{
	R->set_EBP(0);						//start at index 0 instead of 1
	R->set_EBX(0x82B034);				//set the list offset to "TXT_MAP_ARCHIPELAGO"
	return 0x597008;
}

//0x5970EA
DEFINE_HOOK(5970EA, RMG_EnableDesert, 9)
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
DEFINE_HOOK(598FB8, RMG_GenerateUrban, 0)
{
	void* pMapSeed = (void*)R->get_ESI();
	SET_REG32(ecx, pMapSeed);
	CALL(0x5A5020);
}

//0x59000E
DEFINE_HOOK(59000E, RMG_FixPavedRoadEnd_Bridges_North, 0)
{ return 0x590087; }
//0x5900F7
DEFINE_HOOK(5900F7, RMG_FixPavedRoadEnd_Bridges_South, 0)
{ return 0x59015E; }
//0x58FCC6
DEFINE_HOOK(58FCC6, RMG_FixPavedRoadEnd_Bridges_West, 0)
{ return 0x58FD2A; }
//0x58FBDD
DEFINE_HOOK(58FBDD, RMG_FixPavedRoadEnd_Bridges_East, 0)
{ return 0x58FC55; }

//0x58FA51
DEFINE_HOOK(58FA51, RMG_PlaceWEBridge, 6)
{
	RectangleStruct* pRect = (RectangleStruct*)((BYTE*)R->get_ESP()+0x14);

	if(pRect->Width > pRect->Height)	//it's a WE bridge
		return 0x58FA73;
	else
		return 0;
}

//0x58FE7B
DEFINE_HOOK(58FE7B, RMG_PlaceNSBridge, 8)
{
	RectangleStruct* pRect = (RectangleStruct*)((BYTE*)R->get_ESP()+0x14);

	if(pRect->Height > pRect->Width)	//it's a NS bridge
		return 0x58FE91;
	else
		return 0;
}

#ifndef CMD_DEBUGGING_H
#define CMD_DEBUGGING_H

#include "Ares.h"
#include "..\Misc\Debug.h"

#include "..\Ext\Techno\Body.h"
#include "..\Ext\TechnoType\Body.h"
#include "..\Ext\House\Body.h"
#include "..\Ext\HouseType\Body.h"
#include "..\Ext\WeaponType\Body.h"
#include "..\Ext\WarheadType\Body.h"

class DebuggingCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~DebuggingCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Debug Dump"; }

	virtual const wchar_t* GetUIName()
	{ return L"Debugging Dump"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current debug data to the log"; }

	virtual void Execute(DWORD dwUnk)
	{
/*
		Debug::Log("Dumping all Debug data in Frame %X\n\n", Unsorted::CurrentFrame);
		for(int i = 0; i < BuildingClass::Array->Count; ++i) {
			BuildingClass *B = BuildingClass::Array->Items[i];
			CoordStruct *loc = B->get_Location();
			CellStruct XY;
			B->GetMapCoords(&XY);
			Debug::Log("Building [%s] owned by %s\n", 
			B->get_ID(), B->Owner->get_ID());
			//, loc->X, loc->Y, loc->Z, //XY.X, XY.Y, B->GetCurrentMission(), B->FirestormWallFrame, B->LaserFenceFrame);
			if(_strcmpi(B->get_ID(), "GAGAP")) {
				continue;
			}
			byte *W = reinterpret_cast<byte *>(B);
			for(int j = 0; j < 0x71; ++j) {
				Debug::Log("%04X | ", j * 0x10);
				for(int k = 0; k < 0x10; ++k) {
					Debug::Log("%02X ", W[j * 0x10 + k]);
				}
				Debug::Log("\n");
			}
			Debug::Log("\n");
		}
		Debug::Log("All Debug data dumped\n\n");
*/

		MessageListClass::PrintMessage(L"Debug data dumped");
	}

	//Constructor
	DebuggingCommandClass(){}
};

#endif

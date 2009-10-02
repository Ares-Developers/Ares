#include "Ares.h"
#include "Commands\Commands.h"
#include <CommandClass.h>
//include "CallCenter.h"
#include <StaticInits.cpp>

#include <new>

#include "Ext\Building\Body.h"
#include "Ext\BuildingType\Body.h"
//include "Ext\Bullet\Body.h"
#include "Ext\BulletType\Body.h"
#include "Ext\House\Body.h"
#include "Ext\HouseType\Body.h"
#include "Ext\Side\Body.h"
#include "Ext\SWType\Body.h"
#include "Ext\Techno\Body.h"
#include "Ext\TechnoType\Body.h"
#include "Ext\WarheadType\Body.h"
#include "Ext\WeaponType\Body.h"

#include "Misc\Debug.h"
#include "UI\Dialogs.h"

//Init Statics
HANDLE  Ares::hInstance = 0;
bool	Ares::bNoLogo = false;
bool	Ares::bNoCD = false;

DWORD Ares::readLength = BUFLEN;
char Ares::readBuffer[BUFLEN];
const char Ares::readDelims[4] = ",";
const char Ares::readDefval[4] = "";

int Ares::TrackIndex = 66;

int FrameStepCommandClass::ArmageddonState = 0;

stdext::hash_map <DWORD, size_t> MemMap::AllocMap;
size_t MemMap::Total;

//Implementations
eMouseEventFlags __stdcall Ares::MouseEvent(Point2D* pClient,eMouseEventFlags EventFlags)
{
	return EventFlags;
}

void __stdcall Ares::RegisterCommands()
{
	AIControlCommandClass *AICommand;
	GAME_ALLOC(AIControlCommandClass, AICommand);
	CommandClass::Array->AddItem(AICommand);

	MapSnapshotCommandClass *MapSnapshotCommand;
	GAME_ALLOC(MapSnapshotCommandClass, MapSnapshotCommand);
	CommandClass::Array->AddItem(MapSnapshotCommand);

	TestSomethingCommandClass *TestSomethingCommand;
	GAME_ALLOC(TestSomethingCommandClass, TestSomethingCommand);
	CommandClass::Array->AddItem(TestSomethingCommand);

	FrameByFrameCommandClass *FrameByFrameCommand;
	GAME_ALLOC(FrameByFrameCommandClass, FrameByFrameCommand);
	CommandClass::Array->AddItem(FrameByFrameCommand);

	FrameStepCommandClass *FrameStepCommand;
	GAME_ALLOC(FrameStepCommandClass, FrameStepCommand);
	CommandClass::Array->AddItem(FrameStepCommand);

	FirestormToggleCommandClass *FirestormToggleCommand;
	GAME_ALLOC(FirestormToggleCommandClass, FirestormToggleCommand);
	CommandClass::Array->AddItem(FirestormToggleCommand);

	DumperTypesCommandClass *DumperTypesCommand;
	GAME_ALLOC(DumperTypesCommandClass, DumperTypesCommand);
	CommandClass::Array->AddItem(DumperTypesCommand);

	DebuggingCommandClass *DebuggingCommand;
	GAME_ALLOC(DebuggingCommandClass, DebuggingCommand);
	CommandClass::Array->AddItem(DebuggingCommand);

	LoggingCommandClass *LoggingCommand;
	GAME_ALLOC(LoggingCommandClass, LoggingCommand);
	CommandClass::Array->AddItem(LoggingCommand);
}

void __stdcall Ares::CmdLineParse(char** ppArgs,int nNumArgs)
{
	char* pArg;

	Debug::bLog = false;
	bNoCD = false;
	bNoLogo = false;

	// > 1 because the exe path itself counts as an argument, too!
	if(nNumArgs > 1) {
		for(int i = 1; i < nNumArgs; i++) {
			pArg = ppArgs[i];
			_strupr(pArg);
			if(_strcmpi(pArg,"-LOG") == 0) {
				Debug::bLog = true;
			}

			if(_strcmpi(pArg,"-CD") == 0) {
				bNoCD = true;
			}

			if(_strcmpi(pArg,"-NOLOGO") == 0) {
				bNoLogo = true;
			}
		}
	}

	if(!Debug::bLog) {
		Debug::LogFileRemove();
	}
}

void __stdcall Ares::PostGameInit()
{

}

void __stdcall Ares::ExeRun()
{
	Ares::readLength = BUFLEN;
	Debug::LogFileOpen();

	Unsorted::Savegame_Magic = SAVEGAME_MAGIC;
}

void __stdcall Ares::ExeTerminate()
{
	Debug::Log("XTACH\n");
//	Debug::LogFileClose();
}

//A new SendPDPlane function
//Allows vehicles, sends one single plane for all types
void Ares::SendPDPlane(HouseClass* pOwner, CellClass* pTarget, AircraftTypeClass* pPlaneType,
		TypeList<TechnoTypeClass*>* pTypes, TypeList<int>* pNums)
{
	if(pNums && pTypes &&
		pNums->Count == pTypes->Count &&
		pNums->Count > 0 &&
		pOwner && pPlaneType && pTarget)
	{
		++Unsorted::SomeMutex;
		AircraftClass* pPlane = (AircraftClass*)pPlaneType->CreateObject(pOwner);
		--Unsorted::SomeMutex;

		pPlane->Spawned = true;

		//Get edge (direction for plane to come from)
		int edge = pOwner->StartingEdge;
		if(edge < edge_NORTH || edge > edge_WEST) {
			edge = pOwner->Edge;
			if(edge < edge_NORTH || edge > edge_WEST) {
				edge = edge_NORTH;
			}
		}

		//some ASM magic, seems to retrieve a random cell struct at a given edge
		CellStruct spawn_cell;

		MapClass::Global()->PickCellOnEdge(&spawn_cell, edge, (CellStruct *)0xB04C38, (CellStruct *)0xB04C38, 4, 1, 0);

		pPlane->QueueMission(mission_ParadropApproach, false);

		if(pTarget) {
			pPlane->SetTarget(pTarget);
		}

		CoordStruct spawn_crd = {(spawn_cell.X << 8) + 128, (spawn_cell.Y << 8) + 128, 0};

		++Unsorted::SomeMutex;
		bool bSpawned = pPlane->Put(&spawn_crd, dir_N);
		--Unsorted::SomeMutex;

		if(bSpawned) {
			pPlane->HasPassengers = true;
			for(int i = 0; i < pTypes->Count; i++) {
				TechnoTypeClass* pTechnoType = pTypes->GetItem(i);

				//only allow infantry and vehicles
				eAbstractType WhatAmI = pTechnoType->WhatAmI();
				if(WhatAmI == abs_UnitType || WhatAmI == abs_InfantryType) {
					for(int k = 0; k < pNums->Items[i]; k++) {
						FootClass* pNew = (FootClass*)pTechnoType->CreateObject(pOwner);
						pNew->Remove();
						pPlane->get_Passengers()->AddPassenger(pNew);
					}
				}
			}
			pPlane->NextMission();
		} else {
			if(pPlane) {
				GAME_DEALLOC(pPlane);
			}
		}
	}
}

//DllMain
bool __stdcall DllMain(HANDLE hInstance,DWORD dwReason,LPVOID v)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			Ares::hInstance = hInstance;
			Debug::LogFileOpen();
			Debug::Log("ATTACH\n");
			break;
		case DLL_PROCESS_DETACH:
			Debug::Log("DETACH\n");
			Debug::LogFileClose();
			break;
	}

	return true;
}

//Exports

//Hook at 0x52C5E0
DEFINE_HOOK(52C5E0, Ares_NoLogo, 7)
{
	if(Ares::bNoLogo)
		return 0x52C5F3;
	else
		return 0;
}

//0x6AD0ED
DEFINE_HOOK(6AD0ED, Ares_AllowSinglePlay, 5)
{
	return 0x6AD16C;
}

/*
	// 55AFB3, 6
A_FINE_HOOK(55AFB3, Armageddon_Advance, 6)
{
	switch(FrameStepCommandClass::ArmageddonState)
	{
		case 1:
			Unsorted::ArmageddonMode = 0;
			FrameStepCommandClass::ArmageddonState = 2;
			break;
		case 2:
			Unsorted::ArmageddonMode = 1;
			FrameStepCommandClass::ArmageddonState = 0;
			break;
		default:
			break;
	}
	return 0;
}
*/

DEFINE_HOOK(7CD810, ExeRun, 9)
{
	Ares::ExeRun();
	return 0;
}

DEFINE_HOOK(7CD8EF, ExeTerminate, 9)
{
	Ares::ExeTerminate();
	return 0;
}

DEFINE_HOOK(52CAE9, _YR_PostGameInit, 5)
{
	Ares::PostGameInit();
	return 0;
}

DEFINE_HOOK(52F639, _YR_CmdLineParse, 5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Ares::CmdLineParse(ppArgs,nNumArgs);
	return 0;
}

DEFINE_HOOK(533058, CommandClassCallback_Register, 7)
{
	Ares::RegisterCommands();

	DWORD *D;
	GAME_ALLOC(DWORD, D);
	R->set_EAX((DWORD)D);	//Allocate SetUnitTabCommandClass
	return 0x533062;
}


DEFINE_HOOK(7258D0, AnnounceInvalidPointer, 6)
{
	GET(void *, DEATH, ECX);

	TechnoExt::PointerGotInvalid(DEATH);
	TechnoTypeExt::PointerGotInvalid(DEATH);
	WarheadTypeExt::PointerGotInvalid(DEATH);
	WeaponTypeExt::PointerGotInvalid(DEATH);

	return 0;
}

DEFINE_HOOK(74FDC0, GetModuleVersion, 5)
{
	R->set_EAX((DWORD)(VERSION_INTERNAL));
	return 0x74FEEF;
}

DEFINE_HOOK(74FAE0, GetModuleInternalVersion, 5)
{
	R->set_EAX((DWORD)(VERSION_STRMINI));
	return 0x74FC7B;
}

DEFINE_HOOK(685659, Scenario_ClearClasses, a)
{
	BuildingExt::ExtMap.Empty();
	BuildingTypeExt::ExtMap.Empty();
//	BulletExt::ExtMap.Empty();
	BulletTypeExt::ExtMap.Empty();
	HouseExt::ExtMap.Empty();
	HouseTypeExt::ExtMap.Empty();
	SideExt::ExtMap.Empty();
	SWTypeExt::ExtMap.Empty();
	TechnoExt::ExtMap.Empty();
	TechnoTypeExt::ExtMap.Empty();
	WarheadTypeExt::ExtMap.Empty();
	WeaponTypeExt::ExtMap.Empty();

	return 0;
}

/*
A_FINE_HOOK(7C8E17, operator_new, 6)
{
	static int tick = 0;
	GET_STACK(size_t, sz, 0x4);

	void * p = operator new(sz, std::nothrow);
	if(!p) {
		Debug::Log("Boom! 0x%X bytes allocated, failed to alloc 0x%X more\n", MemMap::Total, sz);
		exit(1);
	}

#ifdef MEMORY_LOGGING
	++tick;

	if((tick % 1) == 0) {
		Debug::Log("@ 0x%X: 0x%X + 0x%X bytes\n", R->get_StackVar32(0x0), MemMap::Total, sz);
	}

	MemMap::Add(p, sz);
#endif

	R->set_EAX((DWORD)p);
	return 0x7C8E24;
}

A_FINE_HOOK(7C8B3D, operator_delete, 9)
{
	static int tick = 0;
	GET_STACK(void *, p, 0x4);

#ifdef MEMORY_LOGGING
	size_t sz = MemMap::Remove(p);

	++tick;

	if((tick % 1) == 0) {
		Debug::Log("@ 0x%X: 0x%X - 0x%X bytes\n", R->get_StackVar32(0x0), MemMap::Total, sz);
	}
#endif

	operator delete(p);
	return 0x7C8B47;
}
*/

void Ares::FatalError(const char *Message) {
	MouseClass *ReallyAMap = reinterpret_cast<MouseClass *>(MapClass::Global());
	ReallyAMap->SetPointer(0, 0);
	WWMouseClass::Instance->ReleaseMouse();

	ShowCursor(1);

	strncpy(Dialogs::ExceptDetailedMessage, Message, 0x400);

	Debug::Log("\nFatal Error:\n");
	Debug::Log(Message);

	LPCDLGTEMPLATEA DialogBox = reinterpret_cast<LPCDLGTEMPLATEA>(Game::GetResource(247, 5));

	DialogBoxIndirectParamA(Game::hInstance, DialogBox, Game::hWnd, &Ares::FatalDialog_WndProc, 0);

	Debug::Log("Exiting...\n");
	exit(1);
}

int __stdcall Ares::FatalDialog_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(uMsg > WM_COMMAND) {
		if(uMsg == WM_MOVING) {
			Game::sub_776D80((tagRECT *)lParam);
		}
		return 0;
	}
	switch(uMsg) {
		case WM_COMMAND:
			if(wParam == 1153) {
				EndDialog(hWnd, 1153);
			}
			return 0;
		case WM_CLOSE:
			EndDialog(hWnd, 1153);
			Game::sub_53E420(hWnd);
			return 0;
		case WM_INITDIALOG:
			SetDlgItemTextA(hWnd, Dialogs::ExceptControlID, Dialogs::ExceptDetailedMessage);
			SetFocus(hWnd);
			if ( Game::hWnd ) {
				Game::CenterWindowIn(hWnd, Game::hWnd);
			}
			ShowWindow(hWnd, 1);
			Game::sub_53E3C0(hWnd);
		default:
			return 0;
	}
}


DEFINE_HOOK(4C850B, Exception_Dialog, 5)
{
	MouseClass *ReallyAMap = reinterpret_cast<MouseClass *>(MapClass::Global());
	ReallyAMap->SetPointer(0, 0);
	WWMouseClass::Instance->ReleaseMouse();

	return 0;
}

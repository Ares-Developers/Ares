#include "Ares.h"
#include "Ares.version.h"
#include "Ares.CRT.h"
#include "Commands/Commands.h"
#include <CommandClass.h>
//include "CallCenter.h"
#include <StaticInits.cpp>
#include <Unsorted.h>
#include <GetCDClass.h>

#include <new>

#include "Ext/Building/Body.h"
#include "Ext/BuildingType/Body.h"
#include "Ext/Bullet/Body.h"
#include "Ext/BulletType/Body.h"
#include "Ext/House/Body.h"
#include "Ext/HouseType/Body.h"
#include "Ext/Infantry/Body.h"
#include "Ext/Rules/Body.h"
#include "Ext/Side/Body.h"
#include "Ext/SWType/Body.h"
#include "Ext/Techno/Body.h"
#include "Ext/TechnoType/Body.h"
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

#include "Misc/Debug.h"
#include "Misc/EMPulse.h"

//Init Statics
HANDLE Ares::hInstance = 0;
PVOID Ares::pExceptionHandler = nullptr;
bool Ares::bNoLogo = false;
bool Ares::bNoCD = false;
bool Ares::bTestingRun = false;
bool Ares::bStrictParser = false;
bool Ares::bAllowAIControl = false;
bool Ares::bFPSCounter = false;
bool Ares::bStable = false;
bool Ares::bStableNotification = false;

DWORD Ares::readLength = BUFLEN;
char Ares::readBuffer[BUFLEN];
const char Ares::readDelims[4] = ",";
const char Ares::readDefval[4] = "";

const wchar_t Ares::StabilityWarning[BUFLEN] = L"This version of Ares (" VERSION_WSTR L") is not considered stable.";

MixFileClass *Ares::aresMIX = NULL;

hash_map <DWORD, size_t> MemMap::AllocMap;
size_t MemMap::Total;

void Ares::InitOwnResources()
{
	UninitOwnResources();
	GAME_ALLOC(MixFileClass, aresMIX, "ares.mix");
}

void Ares::UninitOwnResources()
{
	if(aresMIX) {
		GAME_DEALLOC(aresMIX);
		aresMIX = NULL;
	}
}

//Implementations
void __stdcall Ares::RegisterCommands()
{
	if(bAllowAIControl) {
		MakeCommand<AIControlCommandClass>();
	}
	MakeCommand<MapSnapshotCommandClass>();
	MakeCommand<TestSomethingCommandClass>();
	MakeCommand<DumperTypesCommandClass>();
	MakeCommand<MemoryDumperCommandClass>();
	MakeCommand<DebuggingCommandClass>();
	MakeCommand<AIBasePlanCommandClass>();
	MakeCommand<FPSCounterCommandClass>();
}

void __stdcall Ares::CmdLineParse(char** ppArgs,int nNumArgs)
{
	char* pArg;

	Debug::bLog = false;
	bNoCD = false;
	bNoLogo = false;
	EMPulse::verbose = false;

	// > 1 because the exe path itself counts as an argument, too!
	if(nNumArgs > 1) {
		for(int i = 1; i < nNumArgs; i++) {
			pArg = ppArgs[i];
			_strupr(pArg);

			if(strcmp(pArg,"-LOG") == 0) {
				Debug::bLog = true;
			} else if(strcmp(pArg,"-CD") == 0) {
				bNoCD = true;
			} else if(strcmp(pArg,"-NOLOGO") == 0) {
				bNoLogo = true;
			} else if(strcmp(pArg, "-TESTRUN") == 0) {
				bTestingRun = true;
			} else if(strcmp(pArg, "-STRICT") == 0) {
				bStrictParser = true;
			} else if(strcmp(pArg, "-LOG-EMP") == 0) {
				EMPulse::verbose = true;
			} else if(strcmp(pArg,"-AI-CONTROL") == 0) {
				bAllowAIControl = true;
			}
		}
	}

	if(Debug::bLog) {
		Debug::LogFileOpen();
		Debug::Log("Initialized " VERSION_STRVER "\n");
	}

	InitNoCDMode();
}

void __stdcall Ares::PostGameInit()
{

}

void __stdcall Ares::ExeRun()
{
	Ares::readLength = BUFLEN;

	Unsorted::Savegame_Magic = SAVEGAME_MAGIC;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

#if _MSC_VER >= 1700
	// install a new exception handler, if this version of Windows supports it
	if(HINSTANCE handle = GetModuleHandle(TEXT("kernel32.dll"))) {
		if(GetProcAddress(handle, "AddVectoredExceptionHandler")) {
			Ares::pExceptionHandler = AddVectoredExceptionHandler(1, Debug::ExceptionFilter);
		}
	}
#endif
}

void __stdcall Ares::ExeTerminate()
{
	CloseConfig(&Ares::GlobalControls::INI);
	Debug::LogFileClose(111);

	if(Ares::pExceptionHandler) {
		RemoveVectoredExceptionHandler(Ares::pExceptionHandler);
		Ares::pExceptionHandler = nullptr;
	}
}

CCINIClass* Ares::OpenConfig(const char* file) {
	CCINIClass* pINI = NULL;
	GAME_ALLOC(CCINIClass, pINI);

	if(pINI) {
		CCFileClass *cfg = NULL;
		GAME_ALLOC(CCFileClass, cfg, file);

		if(cfg) {
			if(cfg->Exists(NULL)) {
				pINI->ReadCCFile(cfg);
			}
			GAME_DEALLOC(cfg);
		}
	}

	return pINI;
}

void Ares::InitNoCDMode() {
	if(!GetCDClass::Instance->Count) {
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		bNoCD = true;
	}
	
	if(bNoCD) {
		Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
		memset(GetCDClass::Instance->Drives, -1, 26);

		char drv[] = "a:\\";
		for(int i=0; i<26; ++i) {
			drv[0] = 'a' + (i + 2) % 26;
			if(GetDriveTypeA(drv) == DRIVE_FIXED) {
				GetCDClass::Instance->Drives[0] = (i + 2) % 26;
				GetCDClass::Instance->Count = 1;
				break;
			}
		}
	}
}

void Ares::CloseConfig(CCINIClass** ppINI) {
	if(ppINI && *ppINI) {
		GAME_DEALLOC(*ppINI);
		*ppINI = NULL;
	}
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
		++Unsorted::IKnowWhatImDoing;
		AircraftClass* pPlane = reinterpret_cast<AircraftClass*>(pPlaneType->CreateObject(pOwner));
		--Unsorted::IKnowWhatImDoing;

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

		MapClass::Instance->PickCellOnEdge(&spawn_cell, edge, (CellStruct *)0xB04C38, (CellStruct *)0xB04C38, 4, 1, 0);

		pPlane->QueueMission(mission_ParadropApproach, false);

		if(pTarget) {
			pPlane->SetTarget(pTarget);
		}

		CoordStruct spawn_crd = {(spawn_cell.X << 8) + 128, (spawn_cell.Y << 8) + 128, 0};

		++Unsorted::IKnowWhatImDoing;
		bool bSpawned = pPlane->Put(&spawn_crd, Direction::North);
		--Unsorted::IKnowWhatImDoing;

		if(bSpawned) {
			pPlane->HasPassengers = true;
			for(int i = 0; i < pTypes->Count; i++) {
				TechnoTypeClass* pTechnoType = pTypes->GetItem(i);

				//only allow infantry and vehicles
				eAbstractType WhatAmI = pTechnoType->WhatAmI();
				if(WhatAmI == abs_UnitType || WhatAmI == abs_InfantryType) {
					for(int k = 0; k < pNums->Items[i]; k++) {
						FootClass* pNew = reinterpret_cast<FootClass*>(pTechnoType->CreateObject(pOwner));
						pNew->Remove();
						pPlane->Passengers.AddPassenger(pNew);
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
//			Debug::LogFileOpen();
//			Debug::Log("ATTACH\n");
			Debug::LogFileRemove();
			break;
		case DLL_PROCESS_DETACH:
//			Debug::Log("DETACH\n");
//			Debug::LogFileClose();
			break;
	}

	return true;
}

//Exports

//Hook at 0x52C5E0
DEFINE_HOOK(52C5E0, Ares_NoLogo, 7)
{
	return (Ares::bNoLogo)
		? 0x52C5F3
		: 0
	;
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
	GET(int, result, EAX);
	ExitProcess(result); //teehee
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
	R->EAX<DWORD *>(D);	//Allocate SetUnitTabCommandClass
	return 0x533062;
}


DEFINE_HOOK(7258D0, AnnounceInvalidPointer, 6)
{
	GET(void *, DEATH, ECX);

//	Debug::Log("PointerGotInvalid: %X\n", DEATH);

	BuildingExt::ExtMap.PointerGotInvalid(DEATH);
	BuildingTypeExt::ExtMap.PointerGotInvalid(DEATH);
	BulletExt::ExtMap.PointerGotInvalid(DEATH);
	BulletTypeExt::ExtMap.PointerGotInvalid(DEATH);
	HouseExt::ExtMap.PointerGotInvalid(DEATH);
	HouseTypeExt::ExtMap.PointerGotInvalid(DEATH);
	InfantryExt::ExtMap.PointerGotInvalid(DEATH);
	SideExt::ExtMap.PointerGotInvalid(DEATH);
	SWTypeExt::ExtMap.PointerGotInvalid(DEATH);
	TechnoExt::ExtMap.PointerGotInvalid(DEATH);
	TechnoTypeExt::ExtMap.PointerGotInvalid(DEATH);
	WarheadTypeExt::ExtMap.PointerGotInvalid(DEATH);
	WeaponTypeExt::ExtMap.PointerGotInvalid(DEATH);

	return 0;
}

DEFINE_HOOK(685659, Scenario_ClearClasses, a)
{
	BuildingExt::ExtMap.Empty();
	BuildingExt::Cleanup();
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

	RulesExt::ClearCameos();

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

DEFINE_HOOK(47AE36, CDFileClass_SetFileName, 8)
{
	GET(void*, CDControl, EAX);

	if(!CDControl || Ares::bNoCD) {
		return 0x47AEF0;
	}
	return 0x47AE3E;
}

DEFINE_HOOK(47B026, FileFindOpen, 8)
{
	GET(void*, CDControl, EBX);

	if(!CDControl || Ares::bNoCD) {
		return 0x47B0AE;
	}
	return 0x47B02E;
}

bool Ares::RunningOnWindows7OrVista() {
	static bool W7 = false;
	static bool Checked = false;
	if(!Checked) {
		Checked = true;
		OSVERSIONINFO osvi;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		W7 = (osvi.dwMajorVersion == 6)/* && (osvi.dwMinorVersion >= 1)*/;
	}
	return W7;
}

void Ares::UpdateStability() {
	if(Ares::bStable) {
		return;
	}
	if(Unsorted::CurrentFrame < 900) {
		return;
	}
	if(!Ares::bStableNotification) {
		Debug::FatalErrorAndExit("This version of Ares is not considered stable, but for some reason the warning text isn't showing.\n"
			"This suggests that your version of Ares has been tampered with "
			"and the original developers cannot be held responsible for any problems you might experience.");
	}
}

#define YR_SIZE_1000 0x496110
#define YR_SIZE_1001 0x497110
#define YR_SIZE_1001_UC 0x497FE0
#define YR_SIZE_NPATCH 0x5AB000

#define YR_TIME_1000 0x3B846665
#define YR_TIME_1001 0x3BDF544E

#define YR_CRC_1000 0xB701D792
#define YR_CRC_1001_CD 0x098465B3
#define YR_CRC_1001_TFD 0xEB903080
#define YR_CRC_1001_UC 0x1B499086

SYRINGE_HANDSHAKE(pInfo)
{
	if(pInfo) {
		const char* AcceptMsg = "Found Yuri's Revenge %s. Applying Ares " PRODUCT_STR ".";
		const char* PatchDetectedMessage = "Found %s. Ares " PRODUCT_STR " is not compatible with other patches.";

		const char* desc = nullptr;
		const char* msg = nullptr;
		bool allowed = false;

		// accept tfd and cd version 1.001
		if(pInfo->exeTimestamp == YR_TIME_1001) {

			// don't accept expanded exes
			switch(pInfo->exeFilesize) {
			case YR_SIZE_1001:
			case YR_SIZE_1001_UC:

				// all versions allowed
				switch(pInfo->exeCRC) {
				case YR_CRC_1001_CD:
					desc = "1.001 (CD)";
					break;
				case YR_CRC_1001_TFD:
					desc = "1.001 (TFD)";
					break;
				case YR_CRC_1001_UC:
					desc = "1.001 (UC)";
					break;
				default:
					// no-cd, network port or other changes
					desc = "1.001 (modified)";
				}
				msg = AcceptMsg;
				allowed = true;
				break;

			case YR_SIZE_NPATCH:
				// known patch size
				desc = "RockPatch or an NPatch-derived patch";
				msg = PatchDetectedMessage;
				break;

			default:
				// expanded exe, unknown make
				desc = "an unknown game patch";
				msg = PatchDetectedMessage;
			}
		} else if(pInfo->exeTimestamp == YR_TIME_1000) {
			// upgrade advice for version 1.000
			desc = "1.000";
			msg = "Found Yuri's Revenge 1.000 but Ares " PRODUCT_STR " requires version 1.001. Please update your copy of Yuri's Revenge first.";
		} else {
			// does not even compute...
			msg = "Unknown executable. Ares " PRODUCT_STR " requires Command & Conquer Yuri's Revenge version 1.001 (gamemd.exe).";
		}

		// generate the output message
		if(pInfo->Message) {
			sprintf_s(pInfo->Message, pInfo->cchMessage, msg, desc);
		}

		return allowed ? S_OK : S_FALSE;
	}

	return E_POINTER;
}

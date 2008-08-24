#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include "Ares.h"
#include "Commands.h"
#include <CommandClass.h>
#include "CallCenter.h"
#include <StaticInits.cpp>

//Init Statics
bool	Ares::bNoLogo=false;
bool	Ares::bNoCD=false;
bool	Ares::bLog=true;
FILE*	Ares::pLogFile=NULL;

void (_cdecl* Ares::Log)(const char* pFormat,...) = 
	(void (__cdecl *)(const char *,...))0x4068E0;

//Implementations
eMouseEventFlags __stdcall Ares::MouseEvent(Point2D* pClient,eMouseEventFlags EventFlags)
{
	return EventFlags;
}

void __stdcall Ares::RegisterCommands()
{
	CommandClass::Array->AddItem(new AIControlCommandClass());
	CommandClass::Array->AddItem(new MapSnapshotCommandClass());
	CommandClass::Array->AddItem(new TestSomethingCommandClass());
}

void __stdcall Ares::CmdLineParse(char** ppArgs,int nNumArgs)
{
	char* pArg;

	if(nNumArgs>1)	//>1 because the exe path itself counts as an argument, too!
	{
		for(int i=1;i<nNumArgs;i++)
		{
			pArg=ppArgs[i];

			_strupr(pArg);

			if(_strcmpi(pArg,"-LOG")==0)bLog=true;
			if(_strcmpi(pArg,"-CD")==0)bNoCD=true;
			if(_strcmpi(pArg,"-NOLOGO")==0)bNoLogo=true;
		}
	}
	else
	{
		bLog=false;
		bNoCD=false;
		bNoLogo=false;
	}

	if(!bLog)
	{
		LogFile_Close();
		remove("DEBUG.TXT");
	}
}

void __stdcall Ares::PostGameInit()
{

}

void __stdcall Ares::ExeRun()
{
	Ares::LogFile_Open();
}

void __stdcall Ares::ExeTerminate()
{
	Ares::LogFile_Close();
}

//A new SendPDPlane function
//Allows vehicles, sends one single plane for all types
void Ares::SendPDPlane(
		HouseClass* pOwner,
		CellClass* pTarget,
		AircraftTypeClass* pPlaneType,
		TypeList<TechnoTypeClass*>* pTypes,
		TypeList<int>* pNums)
{
	if(pNums && pTypes &&
		pNums->get_Count() == pTypes->get_Count() &&
		pNums->get_Count() > 0 &&
		pOwner && pPlaneType && pTarget)
	{
		*((DWORD*)0xA8E7AC) += 1;		//some mutex
		AircraftClass* pPlane = (AircraftClass*)pPlaneType->CreateObject(pOwner);
		*((DWORD*)0xA8E7AC) -= 1;		//some mutex

		pPlane->set_Spawned(true);

		//Get edge (direction for plane to come from)
		int edge = pOwner->get_StartingEdge();
		if(edge < edge_NORTH || edge > edge_WEST)
		{
			edge = pOwner->get_Edge();
			if(edge < edge_NORTH || edge > edge_WEST)
				edge = edge_NORTH;
		}

		//some ASM magic, seems to retrieve a random cell struct at a given edge
		CellStruct spawn_cell;

		PUSH_IMM(0);	//???
		PUSH_IMM(1);	//???
		PUSH_IMM(4);	//???
		PUSH_IMM(0xB04C38);	//pointer to a null CellStruct
		PUSH_IMM(0xB04C38);
		PUSH_VAR32(edge);
		PUSH_PTR(spawn_cell);
		SET_REG32(ecx, 0x87F7E8);	//MapClass::Global()
		CALL(0x4AA440);

		Ares::Log("Ares: SpawnCell = %d, %d\n", spawn_cell.X, spawn_cell.Y);
		Ares::Log("Ares: pDestination = %d, %d\n", pTarget->get_MapCoords()->X, pTarget->get_MapCoords()->Y);

		pPlane->QueueMission(mission_ParadropApproach, false);

		if(pTarget)
			pPlane->SetTarget(pTarget);

		CoordStruct spawn_crd = {(spawn_cell.X << 8) + 128, (spawn_cell.Y << 8) + 128, 0};

		*((DWORD*)0xA8E7AC) += 1;		//some mutex
		bool bSpawned = pPlane->Put(&spawn_crd, dir_N);
		*((DWORD*)0xA8E7AC) -= 1;		//some mutex

		if(bSpawned)
		{
			pPlane->set_HasPassengers(true);
			for(int i = 0; i < pTypes->get_Count(); i++)
			{
				TechnoTypeClass* pTechnoType = (*pTypes)[i];

				//only allow infantry and vehicles
				if(pTechnoType->WhatAmI() == abs_UnitType ||
					pTechnoType->WhatAmI() == abs_InfantryType)
				{
					for(int k = 0; k < (*pNums)[i]; k++)
					{
						TechnoClass* pNew = (TechnoClass*)pTechnoType->CreateObject(pOwner);
						pNew->vt_entry_D4();
						pPlane->get_Passengers()->AddPassenger(pNew);
					}
				}
			}
			pPlane->NextMission();
		}
		else
			if(pPlane)
				delete pPlane;
	}
}

//DllMain
bool __stdcall DllMain(HANDLE hInstance,DWORD dwReason,LPVOID v)
{
	if(dwReason==DLL_PROCESS_ATTACH)
	{
		CallCenter::Init();
	}

	return true;
}

//Exports

//Hook at 0x4068E0 AND 4A4AC0
EXPORT Ares_Log(REGISTERS* R)
{
	if(Ares::bLog && Ares::pLogFile)
	{
		va_list ArgList=(va_list)(R->get_ESP()+0x8);
		char* Format=(char*)R->get_StackVar32(0x4);

		vfprintf(Ares::pLogFile,Format,ArgList);
		fflush(Ares::pLogFile);
	}
	return 0;
}

//Hook at 0x52C5E0
EXPORT Ares_NoLogo(REGISTERS* R)
{
	if(Ares::bNoLogo)
		return 0x52C5F3;
	else
		return 0;
}

//0x6AD0ED
EXPORT Ares_AllowSinglePlay(REGISTERS* R)
{
	return 0x6AD16C;
}

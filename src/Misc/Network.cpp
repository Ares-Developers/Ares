#include "Network.h"

#include "Debug.h"
#include "Exception.h"
#include "../Ares.h"

#include "../Ext/Building/Body.h"
#include "../Ext/House/Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <NetworkEvents.h>

DEFINE_HOOK(4C6CCD, Networking_RespondToEvent, 0)
{
	GET(DWORD, EventKind, EAX);
	GET(NetworkEvent *, Event, ESI);

	auto kind = static_cast<AresNetEvent::Events>(EventKind);
	if(kind >= AresNetEvent::Events::First) {
		// Received Ares event, do something about it
		switch(kind) {
			case AresNetEvent::Events::TrenchRedirectClick:
				AresNetEvent::Handlers::RespondToTrenchRedirectClick(Event);
				break;
			case AresNetEvent::Events::FirewallToggle:
				AresNetEvent::Handlers::RespondToFirewallToggle(Event);
				break;
		}
	}

	--EventKind;
	R->EAX(EventKind);
	return (EventKind > 0x2D)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}


DEFINE_HOOK(64CCBF, DoList_ReplaceReconMessage, 6)
{
	// mimic an increment because decrement happens in the middle of function cleanup and can't be erased nicely
	int &TempMutex = *reinterpret_cast<int*>(0xA8DAB4);
	++TempMutex;

	Debug::Log("Reconnection error detected!");
	if(MessageBoxW(Game::hWnd, L"Yuri's Revenge has detected a desynchronization!\n"
			L"Would you like to create a full error report for the developers?\n"
			L"Be advised that reports from at least two players are needed.", L"Reconnection Error!", MB_YESNO | MB_ICONERROR) == IDYES) {
		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		std::wstring path = Exception::PrepareSnapshotDirectory();

		if(Debug::bLog) {
			Debug::Log("Copying debug log\n");
			std::wstring logCopy = path + L"\\debug.log";
			CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
		}

		Debug::Log("Making a memory snapshot\n");
		Debug::FullDump(std::move(path));

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);
		Debug::FatalError("A desynchronization has occurred.\r\n"
			"%s"
			"A crash dump should have been created in your game's \\debug subfolder.\r\n"
			"Please submit that to the developers along with SYNC*.txt, debug.txt and syringe.log."
				, Debug::bParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
		);
	}

	return 0x64CD11;
}


/*
 how to raise your own events
	NetworkEvent Event;
	Event.Kind = AresNetworkEvent::aev_blah;
	Event.HouseIndex = U->Owner->ArrayIndex;
	memcpy(Event.ExtraData, "Boom de yada", 0xkcd);
	Networking::AddEvent(&Event);
*/

void AresNetEvent::Handlers::RaiseTrenchRedirectClick(BuildingClass *Source, CellStruct *Target) {
	NetworkEvent Event;
	Event.Kind = static_cast<NetworkEvents>(AresNetEvent::Events::TrenchRedirectClick);
	Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	byte *ExtraData = Event.ExtraData;

	NetID SourceObject, TargetCoords;

	TargetCoords.Pack(Target);
	memcpy(ExtraData, &TargetCoords, sizeof(TargetCoords));
	ExtraData += sizeof(TargetCoords);

	SourceObject.Pack(Source);
	memcpy(ExtraData, &SourceObject, sizeof(SourceObject));
	ExtraData += sizeof(SourceObject);

	Networking::AddEvent(&Event);
}

void AresNetEvent::Handlers::RespondToTrenchRedirectClick(NetworkEvent *Event) {
	NetID *ID = reinterpret_cast<NetID *>(Event->ExtraData);
	if(CellClass * pTargetCell = ID->UnpackCell()) {
		++ID;
		if(BuildingClass * pSourceBuilding = ID->UnpackBuilding()) {
			/*
				pSourceBuilding == selected building the soldiers are in
				pTargetCell == cell the user clicked on; event fires only on buildings which showed the enter cursor
			*/
			BuildingExt::ExtData* sourceBuildingExt = BuildingExt::ExtMap.Find(pSourceBuilding);
			BuildingClass* targetBuilding = pTargetCell->GetBuilding();
			sourceBuildingExt->doTraverseTo(targetBuilding); // check has happened before the enter cursor appeared
		}
	}

}

void AresNetEvent::Handlers::RaiseFirewallToggle(HouseClass *Source) {
	NetworkEvent Event;
	Event.Kind = static_cast<NetworkEvents>(AresNetEvent::Events::FirewallToggle);
	Event.HouseIndex = byte(Source->ArrayIndex);

	Networking::AddEvent(&Event);
}

void AresNetEvent::Handlers::RespondToFirewallToggle(NetworkEvent *Event) {
	if(HouseClass * pSourceHouse = HouseClass::Array->GetItem(Event->HouseIndex)) {
		HouseExt::ExtData *pData = HouseExt::ExtMap.Find(pSourceHouse);
		bool FS = pData->FirewallActive;
		FS = !FS;
		pData->SetFirestormState(FS);
	}
}

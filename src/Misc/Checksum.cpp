#include <Checksummer.h>
#include "../Ext/Abstract/Body.h"
#include <AnimClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>
#include <AircraftClass.h>
#include <Networking.h>
#include <ScenarioClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include "../Ares.version.h"

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
template<typename T>
inline void ChecksumItem(const T *it, DWORD &CombinedChecksum) {
	if(it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2) {
		Checksummer Ch;
		it->CalculateChecksum(&Ch);
		if(auto ExtData = AbstractExt::ExtMap.Find(const_cast<T *>(it))) {
			ExtData->LastChecksum = Ch.CurrentValue;
		}
		CombinedChecksum = Ch.CurrentValue + ((CombinedChecksum >> 31) + 2 * CombinedChecksum);
	}
}

template<typename T>
inline void VectorChecksummer(const DynamicVectorClass<T>* Array, DWORD &CombinedChecksum) {
	if(Array) {
		for(auto i = 0; i < Array->Count; ++i) {
			auto it = Array->Items[i];
			ChecksumItem(it, CombinedChecksum);
		}
	}
}

A_FINE_HOOK(64DAC3, Networking_CalculateFrameCRC, 0)
{
	DWORD Checksum(0);
	VectorChecksummer(AbstractClass::Array0, Checksum);
	VectorChecksummer(AbstractTypeClass::Array, Checksum);

	Checksum = ((Checksum >> 31) + 2 * Checksum) + ScenarioClass::Instance->Random.Random();
	Networking::CurrentFrameCRC = Checksum;

	return 0x64DE70;
}
#endif

template<typename T>
void WriteLog(const T* it, int idx, DWORD checksum, FILE * F) {
	fprintf(F, "#%05d:\t%08X", idx, checksum);
}

template<>
void WriteLog(const AbstractClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<void>(it, idx, checksum, F);
	auto abs = it->WhatAmI();
	fprintf(F, "; Abs: %d (%s)", abs, AbstractClass::GetClassName(abs));
}

template<>
void WriteLog(const ObjectClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<AbstractClass>(it, idx, checksum, F);

	char* typeID = "<None>";
	int typeIndex = -1;
	if(auto pType = it->GetType()) {
		typeID = pType->ID;
		typeIndex = pType->GetArrayIndex();
	}

	CoordStruct crd;
	it->GetCoords(&crd);

	CellStruct cell;
	CellClass::Coord2Cell(&crd, &cell);

	fprintf(F, "; Type: %d (%s); Coords: %d,%d,%d (%d,%d); Health: %d; InLimbo: %d",
		typeIndex, typeID, crd.X, crd.Y, crd.Z, cell.X, cell.Y, it->Health, it->InLimbo);
}

template<>
void WriteLog(const MissionClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<ObjectClass>(it, idx, checksum, F);
	fprintf(F, "; Mission: %d; StartTime: %d",
		it->GetCurrentMission(), it->CurrentMissionStartTime);
}

template<>
void WriteLog(const RadioClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<MissionClass>(it, idx, checksum, F);
	//fprintf(F, "; LastCommand: %d", it->LastCommands[0]);
}

template<>
void WriteLog(const TechnoClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<RadioClass>(it, idx, checksum, F);

	const char* targetID = "<None>";
	int targetIndex = -1;
	CoordStruct targetCrd = {-1, -1, -1};
	if(auto pTarget = it->Target) {
		targetID = AbstractClass::GetClassName(pTarget->WhatAmI());
		targetIndex = pTarget->GetArrayIndex();
		pTarget->GetCoords(&targetCrd);
	}

	fprintf(F, "; Facing: %d; Facing2: %d; Target: %s (%d; %d,%d)",
		(int)it->Facing, (int)it->TurretFacing, targetID, targetIndex, targetCrd.X, targetCrd.Y);
}

template<>
void WriteLog(const FootClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<TechnoClass>(it, idx, checksum, F);

	const char* destID = "<None>";
	int destIndex = -1;
	CoordStruct destCrd = {-1, -1, -1};
	if(auto pDest = it->Destination) {
		destID = AbstractClass::GetClassName(pDest->WhatAmI());
		destIndex = pDest->GetArrayIndex();
		pDest->GetCoords(&destCrd);
	}

	fprintf(F, "; Destination: %s (%d; %d,%d)",
		destID, destIndex, destCrd.X, destCrd.Y);
}

template<>
void WriteLog(const InfantryClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<FootClass>(it, idx, checksum, F);
	fprintf(F, "; Speed %d", Game::F2I(it->SpeedPercentage * 256));
}

template<>
void WriteLog(const UnitClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<FootClass>(it, idx, checksum, F);

	auto Loco = it->Locomotor;
	auto accum = Loco->Get_Speed_Accum();
	auto index = Loco->Get_Track_Index();
	auto number = Loco->Get_Track_Number();

	fprintf(F, "; Speed %d; TrackNumber: %d; TrackIndex: %d", accum, number, index);
}

template<>
void WriteLog(const AircraftClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<FootClass>(it, idx, checksum, F);
	fprintf(F, "; Speed %d; Height: %d", Game::F2I(it->SpeedPercentage * 256), it->GetHeight());
}

template<>
void WriteLog(const BuildingClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<TechnoClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const AbstractTypeClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<AbstractClass>(it, idx, checksum, F);
	fprintf(F, "; ID: %s; Name: %s", it->ID, it->Name);
}

template<>
void WriteLog(const HouseClass* it, int idx, DWORD checksum, FILE * F) {
	WriteLog<void>(it, idx, checksum, F);

	auto House = const_cast<HouseClass*>(it);

	fprintf(F, "; CurrentPlayer: %d; ColorScheme: %s; ID: %d; HouseType: %s; Edge: %d; StartingAllies: %d; Startspot: %d,%d; Visionary: %d; MapIsClear: %d; Money: %d",
		it->CurrentPlayer, ColorScheme::Array->GetItem(it->ColorSchemeIndex)->ID,
		it->ArrayIndex, HouseTypeClass::Array->GetItem(it->Type->ArrayIndex)->Name,
		it->Edge, it->StartingAllies, it->StartingCell.X, it->StartingCell.Y, it->Visionary,
		it->MapIsClear, House->Available_Money());
}

// calls WriteLog and appends a newline
template<typename T>
void WriteLogLine(const T* it, int idx, DWORD checksum, FILE * F) {
	WriteLog(it, idx, checksum, F);
	fprintf(F, "\n");
}

template<typename T>
void LogItem(const T* it, int idx, FILE * F) {
	if(it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2) {
		DWORD Checksum(0);
#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
		if(auto ExtData = AbstractExt::ExtMap.Find(const_cast<T *>(it))) {
			Checksum = ExtData->LastChecksum;
		}
#else
		Checksummer Ch;
		it->CalculateChecksum(&Ch);
		Checksum = Ch.CurrentValue;
#endif
		WriteLogLine(it, idx, Checksum, F);
	}
}

template<typename T>
void VectorLogger(const DynamicVectorClass<T> *Array, FILE * F, const char * Label = nullptr) {
	if(Label) {
		fprintf(F, "Checksums for [%s] (%d)\n", Label, Array ? Array->Count : -1);
	}
	if(Array) {
		for(auto i = 0; i < Array->Count; ++i) {
			auto it = Array->Items[i];
			LogItem(it, i, F);
		}
	} else {
		fprintf(F, "Array not initialized yet...\n");
	}
}

template<typename T>
void HouseLogger(const DynamicVectorClass<T> *Array, FILE * F, const char * Label = nullptr) {
	if(Array) {
		for(auto j = 0; j < HouseClass::Array->Count; ++j) {
			auto pHouse = HouseClass::Array->GetItem(j);
			fprintf(F, "-------------------- %s (%d) %s -------------------\n", pHouse->Type->Name, j, Label ? Label : "");

			for(auto i = 0; i < Array->Count; ++i) {
				auto it = Array->Items[i];

				if(it->Owner == pHouse) {
					LogItem(it, i, F);
				}
			}
		}
	} else {
		fprintf(F, "Array not initialized yet...\n");
	}
}

bool LogFrame(const char * LogFilename, NetworkEvent *OffendingEvent = nullptr) {
	if(auto LogFile = fopen(LogFilename, "wt")) {
		std::setvbuf(LogFile, NULL, _IOFBF, 1024 * 1024); // 1024 kb buffer - should be sufficient for whole log

		fprintf(LogFile, "Ares synchronization log (version " VERSION_STR ")\n");

		for(auto ixF = 0; ixF < 0x100; ++ixF) {
			fprintf(LogFile, "CRC[%02X] = %08X\n", ixF, Networking::LatestFramesCRC[ixF]);
		}

		fprintf(LogFile, "My Random Number: %08X\n", ScenarioClass::Instance->Random.Random());
		fprintf(LogFile, "My Frame: %08X\n", Unsorted::CurrentFrame);
		fprintf(LogFile, "Average FPS: %d\n", Game::F2I(FPSCounter::GetAverageFrameRate()));
		fprintf(LogFile, "Max MaxAhead: %d\n", *(int*)0xA8B568);
		fprintf(LogFile, "Latency setting: %d\n", *(int*)0xA8DB9C);
		fprintf(LogFile, "Game speed setting: %d\n", GameOptionsClass::Instance->GameSpeed);
		fprintf(LogFile, "FrameSendRate: %d\n", *(int*)0xA8B554);
		
		if(OffendingEvent) {
			fprintf(LogFile, "\nOffending event:\n");
			fprintf(LogFile, "Type:         %X\n", OffendingEvent->Kind);
			fprintf(LogFile, "Frame:        %X\n", OffendingEvent->Timestamp);
			fprintf(LogFile, "ID:           %X\n", OffendingEvent->HouseIndex);
			fprintf(LogFile, "CRC:          %X\n", OffendingEvent->Checksum);
			fprintf(LogFile, "CommandCount: %hu\n", OffendingEvent->CommandCount);
			fprintf(LogFile, "Delay:        %hhu\n", OffendingEvent->Delay);
			fprintf(LogFile, "\n\n");
		}

		fprintf(LogFile, "\nTypes\n");
		HouseLogger(InfantryClass::Array, LogFile, "Infantry");
		HouseLogger(UnitClass::Array, LogFile, "Units");
		HouseLogger(AircraftClass::Array, LogFile, "Aircraft");
		HouseLogger(BuildingClass::Array, LogFile, "Buildings");

		fprintf(LogFile, "\nChecksums\n");
		VectorLogger(HouseClass::Array, LogFile, "Houses");
		VectorLogger(InfantryClass::Array, LogFile, "Infantry");
		VectorLogger(UnitClass::Array, LogFile, "Units");
		VectorLogger(AircraftClass::Array, LogFile, "Aircraft");
		VectorLogger(BuildingClass::Array, LogFile, "Buildings");

		fprintf(LogFile, "\n");
		VectorLogger(ObjectClass::CurrentObjects, LogFile, "Current Objects");
		VectorLogger(ObjectClass::Logics, LogFile, "Logics");

		fprintf(LogFile, "\nChecksums for Map Layers\n");
		for(auto ixL = 0; ixL < 5; ++ixL) {
			fprintf(LogFile, "Checksums for Layer %d\n", ixL);
			VectorLogger(&(ObjectClass::ObjectsInLayers[ixL]), LogFile);
		}

		fprintf(LogFile, "\nChecksums for Logics\n");
		VectorLogger(ObjectClass::Logics, LogFile);
		
		fprintf(LogFile, "\nChecksums for Abstracts\n");
		VectorLogger(AbstractClass::Array0, LogFile, "Abstracts");
		VectorLogger(AbstractTypeClass::Array, LogFile, "AbstractTypes");

		fclose(LogFile);
		return true;
	} else {
		Debug::Log("Failed to open file for sync log. Error code %X.\n", GetLastError());
		return false;
	}
}

DEFINE_HOOK(64DEA0, Multiplay_LogToSYNC_NOMPDEBUG, 0)
{
	GET(NetworkEvent *, OffendingEvent, ECX);
	
	char LogFilename[0x40];
	_snprintf(LogFilename, sizeof(LogFilename), "SYNC%01d.TXT", HouseClass::Player->ArrayIndex);

	LogFrame(LogFilename, OffendingEvent);

	return 0x64DF3D;
}

DEFINE_HOOK(6516F0, Multiplay_LogToSync_MPDEBUG, 6)
{
	GET(int, SlotNumber, ECX);
	GET(NetworkEvent *, OffendingEvent, EDX);
	
	char LogFilename[0x40];
	_snprintf(LogFilename, sizeof(LogFilename), "SYNC%01d_%03d.TXT", HouseClass::Player->ArrayIndex, SlotNumber);

	LogFrame(LogFilename, OffendingEvent);

	return 0x651781;
}

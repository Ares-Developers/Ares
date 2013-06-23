#include <Checksummer.h>
#include "../Ext/Abstract/Body.h"
#include <AnimClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>
#include <AircraftClass.h>
#include <Networking.h>
#include <ScenarioClass.h>
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
		fprintf(F, "#%05d:\t%08X\n", idx, Checksum);
	}
}

template<typename T>
void VectorLogger(const DynamicVectorClass<T> *Array, FILE * F, const char * Label = nullptr) {
	if(Label) {
		fprintf(F, "Checksums for [%s]\n", Label);
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

bool LogFrame(const char * LogFilename, NetworkEvent *OffendingEvent = nullptr) {
	if(auto LogFile = fopen(LogFilename, "wt")) {
		std::setvbuf(LogFile, NULL, _IOFBF, 1024 * 256); // 256 kb buffer - should be sufficient for whole log

		fprintf(LogFile, "Ares synchronization log (version " VERSION_STR ")\n");

		for(auto ixF = 0; ixF < 0x100; ++ixF) {
			fprintf(LogFile, "CRC[%02X] = %08X\n", ixF, Networking::LatestFramesCRC[ixF]);
		}

		fprintf(LogFile, "My Random Number: %08X\n", ScenarioClass::Instance->Random.Random());
		fprintf(LogFile, "My Frame: %08X\n", Unsorted::CurrentFrame);
		
		if(OffendingEvent) {
			fprintf(LogFile, "\nOffending event:\n");
			fprintf(LogFile, "Type:         %X\n", OffendingEvent->Kind);
			fprintf(LogFile, "Frame:        %X\n", OffendingEvent->Timestamp);
			fprintf(LogFile, "ID:           %X\n", OffendingEvent->HouseIndex);
			fprintf(LogFile, "CRC:          %X\n", OffendingEvent->Checksum);
			fprintf(LogFile, "CommandCount: %d\n", (int)OffendingEvent->CommandCount);
			fprintf(LogFile, "Delay:        %d\n", (int)OffendingEvent->Delay);
			fprintf(LogFile, "\n\n");
		}

		fprintf(LogFile, "Checksums\n");

		VectorLogger(HouseClass::Array, LogFile, "Houses");
		VectorLogger(InfantryClass::Array, LogFile, "Infantry");
		VectorLogger(UnitClass::Array, LogFile, "Units");
		VectorLogger(AircraftClass::Array, LogFile, "Aircraft");
		VectorLogger(BuildingClass::Array, LogFile, "Buildings");

		VectorLogger(ObjectClass::CurrentObjects, LogFile, "Current Objects");
		VectorLogger(ObjectClass::Logics, LogFile, "Logics");

		fprintf(LogFile, "Checksums for Map Layers\n");
		for(auto ixL = 0; ixL < 5; ++ixL) {
			fprintf(LogFile, "Checksums for Layer %d\n", ixL);
			VectorLogger(&(ObjectClass::ObjectsInLayers[ixL]), LogFile);
		}

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

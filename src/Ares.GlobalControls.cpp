#include "Ares.h"
#include <CCINIClass.h>

bool Ares::GlobalControls::Initialized = 0;
bool Ares::GlobalControls::AllowParallelAIQueues = 1;

bool Ares::GlobalControls::DebugKeysEnabled = true;

byte Ares::GlobalControls::GFX_DX_Force = 0;

CCINIClass *Ares::GlobalControls::INI = NULL;

std::bitset<3> Ares::GlobalControls::AllowBypassBuildLimit(0ull);

void Ares::GlobalControls::Load(CCINIClass *pINI) {
	Initialized = 1;
	AllowParallelAIQueues = pINI->ReadBool("GlobalControls", "AllowParallelAIQueues", AllowParallelAIQueues);

	if(pINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Ares::readBuffer, Ares::readLength)) {
		int idx = 0;
		for(char * cur = strtok(Ares::readBuffer, ","); cur && *cur && idx <= 2; cur = strtok(NULL, ","), ++idx) {
			int diffIdx = 2 - idx; // remapping so that HouseClass::AIDifficulty can be used as an index
			switch(toupper(*cur)) {
			case '1':
			case 'T':
			case 'Y':
				AllowBypassBuildLimit[diffIdx] = true;
			default:
				AllowBypassBuildLimit[diffIdx] = false;
			}
		}
	}

	// used by the keyboard commands
	if(pINI == CCINIClass::INI_Rules) {
		DebugKeysEnabled = true;
	}
	DebugKeysEnabled = pINI->ReadBool("GlobalControls", "DebugKeysEnabled", DebugKeysEnabled);
}

void Ares::GlobalControls::LoadConfig() {
	if(INI->ReadString("Graphics.Advanced", "DirectX.Force", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		if(!_strcmpi(Ares::readBuffer, "hardware")) {
			GFX_DX_Force = GFX_DX_HW;
		} else if(!_strcmpi(Ares::readBuffer, "emulation")) {
			GFX_DX_Force = GFX_DX_EM;
		}
	}
	if(Ares::RunningOnWindows7OrVista()) {
		GFX_DX_Force = 0;
	}
}

DEFINE_HOOK(6BC0CD, _LoadRA2MD, 5)
{
	Ares::GlobalControls::INI = Ares::OpenConfig("Ares.ini");
	Ares::GlobalControls::LoadConfig();
	return 0;
}

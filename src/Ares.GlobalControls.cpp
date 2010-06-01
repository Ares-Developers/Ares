#include "Ares.h"
#include <CCINIClass.h>

bool Ares::GlobalControls::Initialized = 0;
bool Ares::GlobalControls::AllowParallelAIQueues = 1;
bool Ares::GlobalControls::AllowMultiEngineer = 0;

byte Ares::GlobalControls::GFX_DX_Force = 0;

CCINIClass *Ares::GlobalControls::INI = NULL;

Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Alternate = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Composite = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden_2 = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Primary = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Sidebar = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Tile = {0xFF, 0xFF};

std::bitset<3> Ares::GlobalControls::AllowBypassBuildLimit(0ull);


//define DDCREATE_HARDWAREONLY  0x00000001l
//define DDCREATE_EMULATIONONLY 0x00000002l

void Ares::GlobalControls::Load(CCINIClass *pINI) {
	Initialized = 1;
	AllowParallelAIQueues = pINI->ReadBool("GlobalControls", "AllowParallelAIQueues", AllowParallelAIQueues);

	if(pINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Ares::readBuffer, Ares::readLength)) {
		int idx = 0;
		for(char * cur = strtok(Ares::readBuffer, ","); cur && *cur && idx <= 2; cur = strtok(NULL, ","), ++idx) {
			int diffIdx = 2 - idx; // remapping so that HouseClass::AIDifficulty can be used as an index
			AllowBypassBuildLimit[diffIdx] = _strcmpi(cur, "Yes") == 0;
		}
	}
}

void Ares::GlobalControls::LoadFromRules(CCINIClass *pINI) {
	AllowMultiEngineer = pINI->ReadBool("General", "AllowMultiEngineer", AllowMultiEngineer);
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

#define ReadSurface(__surface__) \
	if(INI->ReadString("Graphics.Advanced", "Surface." str(__surface__) ".Memory", Ares::readDefval, Ares::readBuffer, Ares::readLength)) { \
		if(!_strcmpi(Ares::readBuffer, "VRAM")) { \
			GFX_S_ ## __surface__ .Memory = GFX_SU_VRAM; \
		} else if(!_strcmpi(Ares::readBuffer, "System")) { \
			GFX_S_ ## __surface__ .Memory = GFX_SU_SYSTEM; \
		} \
	} \
	if(INI->ReadString("Graphics.Advanced", "Surface." str(__surface__) ".Force3D", Ares::readDefval, Ares::readBuffer, Ares::readLength)) { \
	 bool F3D = INI->ReadBool("Graphics.Advanced", "Surface." str(__surface__) ".Force3D", false); \
	 GFX_S_ ## __surface__ .Force3D = F3D ? 1 : 0;\
	}

	ReadSurface(Alternate);
	ReadSurface(Composite);
	ReadSurface(Hidden);
	ReadSurface(Hidden_2);
	ReadSurface(Primary);
	ReadSurface(Sidebar);
	ReadSurface(Tile);
}

DEFINE_HOOK(6BC0CD, _LoadRA2MD, 5)
{
	Ares::GlobalControls::INI = Ares::GlobalControls::OpenConfig("Ares.ini");
	Ares::GlobalControls::LoadConfig();
	return 0;
}

DEFINE_HOOK(5FACDF, _Options_LoadFromINI, 5)
{
	// open the rules file
	Debug::Log("--------- Loading Ares global settings -----------\n");
	CCINIClass *pINI = Ares::GlobalControls::OpenConfig("rulesmd.ini");
	
	// load and output settings
	Ares::GlobalControls::LoadFromRules(pINI);
	Debug::Log("AllowMultiEngineer is %s\n", (Ares::GlobalControls::AllowMultiEngineer ? "ON" : "OFF"));

	// clean up
	Ares::GlobalControls::CloseConfig(&pINI);
	return 0;
}

CCINIClass* Ares::GlobalControls::OpenConfig(const char* file) {
	CCINIClass* INI;
	GAME_ALLOC(CCINIClass, INI);
	CCFileClass *cfg;
	GAME_ALLOC(CCFileClass, cfg, file);
	if(cfg->Exists(NULL)) {
		INI->ReadCCFile(cfg);
	}
	GAME_DEALLOC(cfg);

	return INI;
}

void Ares::GlobalControls::CloseConfig(CCINIClass** ppINI) {
	GAME_DEALLOC(&ppINI);
	*ppINI = NULL;
}

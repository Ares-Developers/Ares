#include "Ares.h"
#include <CCINIClass.h>

bool Ares::GlobalControls::Initialized = 0;
bool Ares::GlobalControls::AllowParallelAIQueues = 1;

byte Ares::GlobalControls::GFX_DX_Force = 0;

CCINIClass *Ares::GlobalControls::INI = NULL;

Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Alternate = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Composite = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden_2 = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Primary = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Sidebar = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Tile = {0xFF, 0xFF};

//define DDCREATE_HARDWAREONLY  0x00000001l
//define DDCREATE_EMULATIONONLY 0x00000002l

void Ares::GlobalControls::Load(CCINIClass *pINI) {
	Initialized = 1;
	AllowParallelAIQueues = pINI->ReadBool("GlobalControls", "AllowParallelAIQueues", AllowParallelAIQueues);
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

//ifndef str
#define str(x) str_(x)
#define str_(x) #x
//endif

#define ReadSurface(__surface__) \
	if(INI->ReadString("Graphics.Advanced", "Surface." str(__surface__) ".Memory", Ares::readDefval, Ares::readBuffer, Ares::readLength)) { \
		if(!_strcmpi(Ares::readBuffer, "VRAM")) { \
			GFX_S_ ## __surface__ .Memory = GFX_SU_VRAM; \
		} else if(!_strcmpi(Ares::readBuffer, "System")) { \
			GFX_S_ ## __surface__ .Memory = GFX_SU_SYSTEM; \
		} \
	} \
	Debug::Log("Surface." str(__surface__) ".Memory = %s/%d\n", Ares::readBuffer, GFX_S_ ## __surface__ .Memory); \
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
	Debug::Log("LRM1\n");
	Ares::GlobalControls::OpenConfig();
	Debug::Log("LRM1+\n");
	Ares::GlobalControls::LoadConfig();
	Debug::Log("LRM2\n");
	return 0;
}

void Ares::GlobalControls::OpenConfig() {
	Debug::Log("OC1\n");
	INI = new CCINIClass();
	Debug::Log("OC2\n");
	CCFileClass *cfg = new CCFileClass("Ares.ini");
	Debug::Log("OC3\n");
	if(cfg->Exists(NULL)) {
		Debug::Log("OC4+\n");
		INI->ReadCCFile(cfg);
	} else {
		Debug::Log("OC4-\n");
		Debug::Log("Ares.ini does not exist.");
		ExitProcess(1);
	}
	Debug::Log("OC5\n");
	delete cfg;
}

void Ares::GlobalControls::CloseConfig() {
	delete INI;
	INI = NULL;
}

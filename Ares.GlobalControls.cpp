#include "Ares.h"
#include <CCINIClass.h>

bool Ares::GlobalControls::Initialized = 0;
bool Ares::GlobalControls::AllowParallelAIQueues = 1;

byte Ares::GlobalControls::GFX_DX_Force = 0xFF;
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Alternate = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Composite = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Hidden_2 = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Primary = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Sidebar = {0xFF, 0xFF};
Ares::GlobalControls::SurfaceConfig Ares::GlobalControls::GFX_S_Tile = {0xFF, 0xFF};

//define DDCREATE_HARDWAREONLY  0x00000001l
//define DDCREATE_EMULATIONONLY 0x00000002l

#define GFX_DX_HW 0x01l
#define GFX_DX_EM 0x02l

#define GFX_SU_VRAM 0x01l
#define GFX_SU_SYSTEM 0x02l

#define GFX_SU_F3D 0x01l
#define GFX_SU_NF3D 0x02l

void Ares::GlobalControls::Load(CCINIClass *pINI) {
	Initialized = 1;
	AllowParallelAIQueues = pINI->ReadBool("GlobalControls", "AllowParallelAIQueues", AllowParallelAIQueues);

#if 0
	CCINIClass *INI = CCINIClass::INI_RA2MD;

	if(INI->ReadString("Graphics.Advanced", "DirectX.Force", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		if(!_strcmpi(Ares::readBuffer, "hardware")) {
			GFX_DX_Force = GFX_DX_HW;
		} else if(!_strcmpi(Ares::readBuffer, "emulation")) {
			GFX_DX_Force = GFX_DX_EM;
		}
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
	GFX_S_ ## __surface__ .Force3D = INI->ReadBool("Graphics.Advanced", "Surface." # str(__surface__) # ".Force3D", GFX_S_ ## __surface__ .Force3D);


	ReadSurface(Alternate);
	ReadSurface(Composite);
	ReadSurface(Hidden);
	ReadSurface(Hidden_2);
	ReadSurface(Primary);
	ReadSurface(Sidebar);
	ReadSurface(Tile);

#endif

}


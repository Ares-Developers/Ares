#include <Drawing.h>
#include <YRDDraw.h>
#include <WWMouseClass.h>

#include "Ares.h"
#include "Utilities/Macro.h"

#include "Ext/TechnoType/Body.h"
#include "Ext/SWType/Body.h"

DEFINE_HOOK(533FD0, AllocateSurfaces, 0)
{
	GET(RectangleStruct *, rect_Hidden, ECX);
	GET(RectangleStruct *, rect_Composite, EDX);
	GET_STACK(RectangleStruct *, rect_Tile, 0x4);
	GET_STACK(RectangleStruct *, rect_Sidebar, 0x8);
	GET_STACK(byte, flag, 0xC);

	RectangleStruct *rect_Alternate = rect_Hidden;

#define DELSURFACE(surface) \
	if(surface) { \
		GAME_DEALLOC(surface); \
		surface = 0; \
	}

	DELSURFACE(DSurface::Alternate);
	DELSURFACE(DSurface::Hidden);
	DELSURFACE(DSurface::Composite);
	DELSURFACE(DSurface::Tile);
	DELSURFACE(DSurface::Sidebar);

#define ALLOCSURFACE(surface, mem, f3d) \
	if(rect_ ## surface->Width > 0 && rect_ ## surface->Height > 0) { \
		Ares::GlobalControls::SurfaceConfig *SConfig = &Ares::GlobalControls::GFX_S_ ## surface; \
		byte Memory =  SConfig->Memory; \
		if(Memory == 0xFF) { \
			Memory = mem; \
		} \
		byte Force3D =  SConfig->Force3D; \
		if(Force3D == 0xFF) { \
			Force3D = f3d; \
		} \
		DSurface *S; \
		GAME_ALLOC(DSurface, S, rect_ ## surface->Width, rect_ ## surface->Height, !!Memory, !!Force3D); \
		if(!S) { \
			Debug::FatalErrorAndExit("Failed to allocate " str(surface) " - cannot continue."); \
		} \
		DSurface:: ## surface = S; \
		S->Fill(0); \
	}

	if(flag) {
		ALLOCSURFACE(Hidden, 0, 0);
	}

	ALLOCSURFACE(Composite, !Game::bVideoBackBuffer, 1);
	ALLOCSURFACE(Tile, !Game::bVideoBackBuffer, 1);

	if(DSurface::Composite->VRAMmed ^ DSurface::Tile->VRAMmed) {
		DELSURFACE(DSurface::Composite);
		DELSURFACE(DSurface::Tile);
		GAME_ALLOC(DSurface, DSurface::Composite, rect_Composite->Width, rect_Composite->Height, 1, 1);
		GAME_ALLOC(DSurface, DSurface::Tile, rect_Tile->Width, rect_Tile->Height, 1, 1);
	}

	ALLOCSURFACE(Sidebar, !Game::bAllowVRAMSidebar, 0);

	if(!flag) {
		ALLOCSURFACE(Hidden, 0, 0);
	}

	ALLOCSURFACE(Alternate, 1, 0);

	return 0x53443E;
}

DEFINE_HOOK(7C89D4, DirectDrawCreate, 6)
{
	R->Stack<DWORD>(0x4, Ares::GlobalControls::GFX_DX_Force);
	return 0;
}


DEFINE_HOOK(7B9510, WWMouseClass_DrawCursor_V1, 6)
DEFINE_HOOK_AGAIN(7B94B2, WWMouseClass_DrawCursor_V2, 6)
{
	void *Blitter = FileSystem::MOUSE_PAL->SelectProperBlitter(WWMouseClass::Instance->Image, WWMouseClass::Instance->ImageFrameIndex, 0);

	R->Stack<void*>(0x18, Blitter);

	return 0;
}

#if 0
A_FINE_HOOK(537CFE, Game_MakeScreenshot, 0)
{
	DSurface * Surface = DSurface::Hidden;
	if(WORD * buffer = reinterpret_cast<WORD *>(Surface->Lock(0, 0))) {

		int idx = -1;
		char fname[16];
		CCFileClass *ScreenShot = new CCFileClass;
		do {
			++idx;
			_snprintf(fname, 16, "SCRN%04.bmp", idx);
		} while(ScreenShot->Exists(fname));

		ScreenShot->OpenEx(fname, eFileMode::Write);

		int width = Surface->GetWidth();
		int height = Surface->GetHeight();

		size_t arrayLen = width * height;

		#pragma pack(push, 1)
		struct bmpfile_magic {
		  unsigned char magic[2];
		} h1;

		h1.magic[0] = 'B';
		h1.magic[1] = 'M';

		struct bmpfile_header {
		  DWORD filesz;
		  WORD creator1;
		  WORD creator2;
		  DWORD bmp_offset;
		} h2;
		h2.creator1 = h2.creator2 = 0;

		struct bmp_dib_v3_header_t {
		  DWORD header_sz;
		  DWORD width;
		  DWORD height;
		  WORD nplanes;
		  WORD bitspp;
		  DWORD compress_type;
		  DWORD bmp_bytesz;
		  DWORD hres;
		  DWORD vres;
		  DWORD ncolors;
		  DWORD nimpcolors;
		} h3;
		h3.header_sz = 40;
		h3.width = width;
		h3.height = -height; // magic! no need to reverse rows this way
		h3.nplanes = 1;
		h3.bitspp = 16;
		h3.compress_type = BI_BITFIELDS;
		h3.bmp_bytesz = arrayLen * 2;
		h3.hres = 4000;
		h3.vres = 4000;
		h3.ncolors = h3.nimpcolors = 0;

		struct bmp_rgbmask {
			DWORD R; //
			DWORD G; //
			DWORD B; //
		} h4;
		h4.R = 0xF800;
		h4.G = 0x07E0;
		h4.B = 0x001F; // look familiar?

		h2.bmp_offset = sizeof(h1) + sizeof(h2) + sizeof(h3) + sizeof(h4);
		h2.filesz = h2.bmp_offset + h3.bmp_bytesz;

		#pragma pack(pop)

		ScreenShot->WriteBytes(&h1, sizeof(h1));
		ScreenShot->WriteBytes(&h2, sizeof(h2));
		ScreenShot->WriteBytes(&h3, sizeof(h3));
		ScreenShot->WriteBytes(&h4, sizeof(h4));
		WORD *pixels = new WORD [arrayLen];
		WORD *pixelData = pixels;
		int pitch = Surface->SurfDesc->lPitch;
		for(int r = 0; r < height; ++r) {
			memcpy(pixels, reinterpret_cast<void *>(buffer), width * 2);
			pixels += width;
			buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
		}
		Debug::Log("Writing file");
		ScreenShot->WriteBytes(pixelData, arrayLen * 2);
		ScreenShot->Close();
		delete[] pixelData;
	}
	return 0x537DC0;
}
#endif

#include <Drawing.h>
#include <YRDDraw.h>
#include <WWMouseClass.h>

#include "../Ares.h"
#include "../Utilities/Macro.h"

#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

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

DEFINE_HOOK(537BC0, Game_MakeScreenshot, 0)
{
	RECT Viewport;
	if(Imports::GetWindowRect(Game::hWnd, &Viewport)) {
		POINT TL = {Viewport.left, Viewport.top}, BR = {Viewport.right, Viewport.bottom};
		if(Imports::ClientToScreen(Game::hWnd, &TL) && Imports::ClientToScreen(Game::hWnd, &BR)) {
			RectangleStruct ClipRect = {TL.x, TL.y, Viewport.right + 1, Viewport.bottom + 1};

			DSurface * Surface = DSurface::Primary;

			int width = Surface->GetWidth();
			int height = Surface->GetHeight();

			size_t arrayLen = width * height;

			if(width < ClipRect.Width) {
				ClipRect.Width = width;
			}
			if(height < ClipRect.Height) {
				ClipRect.Height = height;
			}

//			RectangleStruct DestRect = {0, 0, width, height};

			WWMouseClass::Instance->HideCursor();
//			Surface->BlitPart(&DestRect, DSurface::Primary, &ClipRect, 0, 1);

			if(WORD * buffer = reinterpret_cast<WORD *>(Surface->Lock(0, 0))) {
				int idx = -1;
				char fname[16];
				CCFileClass *ScreenShot = NULL;
				do {
					if(ScreenShot) {
						delete ScreenShot;
						ScreenShot = NULL;
					}
					++idx;
					_snprintf(fname, 16, "SCRN%04d.bmp", idx);
					ScreenShot = new CCFileClass(fname);
				} while(ScreenShot->Exists(0));

				ScreenShot->OpenEx(fname, eFileMode::Write);

				#pragma pack(push, 1)
				struct bmpfile_full_header {
					unsigned char magic[2];
					DWORD filesz;
					WORD creator1;
					WORD creator2;
					DWORD bmp_offset;
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
					DWORD R; //
					DWORD G; //
					DWORD B; //
				} h;
				#pragma pack(pop)

				h.magic[0] = 'B';
				h.magic[1] = 'M';

				h.creator1 = h.creator2 = 0;

				h.header_sz = 40;
				h.width = width;
				h.height = -height; // magic! no need to reverse rows this way
				h.nplanes = 1;
				h.bitspp = 16;
				h.compress_type = BI_BITFIELDS;
				h.bmp_bytesz = arrayLen * 2;
				h.hres = 4000;
				h.vres = 4000;
				h.ncolors = h.nimpcolors = 0;

				h.R = 0xF800;
				h.G = 0x07E0;
				h.B = 0x001F; // look familiar?

				h.bmp_offset = sizeof(h);
				h.filesz = h.bmp_offset + h.bmp_bytesz;

				ScreenShot->WriteBytes(&h, sizeof(h));
				WORD *pixels = new WORD [arrayLen];
				WORD *pixelData = pixels;
				int pitch = Surface->SurfDesc->lPitch;
				for(int r = 0; r < height; ++r) {
					memcpy(pixels, reinterpret_cast<void *>(buffer), width * 2);
					pixels += width;
					buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
				}

				ScreenShot->WriteBytes(pixelData, arrayLen * 2);
				ScreenShot->Close();
				delete[] pixelData;
				delete ScreenShot;

				Debug::Log("Wrote screenshot to file %s\n", fname);
				Surface->Unlock();
			}

			WWMouseClass::Instance->ShowCursor();

		}
	}

	return 0x537DC9;
}


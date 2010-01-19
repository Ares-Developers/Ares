#include <Drawing.h>
#include <YRDDraw.h>
#include <WWMouseClass.h>

#include "Ares.h"

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
		delete surface; \
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
		DSurface *S = new DSurface(rect_ ## surface->Width, rect_ ## surface->Height, !!Memory, !!Force3D); \
		if(!S) { \
			Debug::FatalError("Failed to allocate " str(surface) " - cannot continue."); \
		} \
		DSurface:: ## surface = S; \
		S->Fill(0); \
	}

	if(flag) {
		ALLOCSURFACE(Hidden, 0, 0);
	}

	ALLOCSURFACE(Composite, Game::bVideoBackBuffer, 1);
	ALLOCSURFACE(Tile, Game::bVideoBackBuffer, 1);

	if(DSurface::Composite->VRAMmed ^ DSurface::Tile->VRAMmed) {
		DELSURFACE(DSurface::Composite);
		DELSURFACE(DSurface::Tile);
		DSurface::Composite = new DSurface(rect_Composite->Width, rect_Composite->Height, 1, 1);
		DSurface::Tile = new DSurface(rect_Tile->Width, rect_Tile->Height, 1, 1);
	}

	ALLOCSURFACE(Sidebar, Game::bAllowVRAMSidebar, 1);

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

#include <Drawing.h>
#include <YRDDraw.h>
#include <WWMouseClass.h>
#include <FPSCounter.h>

#include "../Ares.h"
#include "../Utilities/Macro.h"

#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

DEFINE_HOOK(7C89D4, DirectDrawCreate, 6)
{
	R->Stack<DWORD>(0x4, Ares::GlobalControls::GFX_DX_Force);
	return 0;
}

DEFINE_HOOK(7B9510, WWMouseClass_DrawCursor_V1, 6)
//A_FINE_HOOK_AGAIN(7B94B2, WWMouseClass_DrawCursor_V1, 6)
{
	void *Blitter = FileSystem::MOUSE_PAL->SelectProperBlitter(WWMouseClass::Instance->Image, WWMouseClass::Instance->ImageFrameIndex, 0);

	R->Stack<void*>(0x18, Blitter);

	return 0;
}

DEFINE_HOOK(537BC0, Game_MakeScreenshot, 0)
{
	RECT Viewport = {};
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

				char fName[0x80];

				SYSTEMTIME time;
				GetLocalTime(&time);

				_snprintf(fName, 0x80, "SCRN.%04u%02u%02u-%02u%02u%02u-%05u.BMP",
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

				CCFileClass *ScreenShot = nullptr;
				GAME_ALLOC(CCFileClass, ScreenShot, "\0");

				ScreenShot->OpenEx(fName, eFileMode::Write);

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
				GAME_DEALLOC(ScreenShot);

				Debug::Log("Wrote screenshot to file %s\n", fName);
				Surface->Unlock();
			}

			WWMouseClass::Instance->ShowCursor();

		}
	}

	return 0x537DC9;
}

DEFINE_HOOK(4F4583, GScreenClass_DrawOnTop_TheDarkSideOfTheMoon, 6)
{
	const int AdvCommBarHeight = 32;

	int offset = AdvCommBarHeight;

	auto DrawText = [](const wchar_t* string, int& offset, int color) {
		auto wanted = Drawing::GetTextDimensions(string);

		auto h = DSurface::Composite->GetHeight();
		RectangleStruct rect = {0, h - wanted.Height - offset, wanted.Width, wanted.Height};

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawTextA(string, 0, rect.Y, color);

		offset += wanted.Height;
	};

	if(!Ares::bStable) {
		Ares::bStableNotification = true;
		DrawText(Ares::StabilityWarning, offset, COLOR_RED);
	}

	static const CSFText modnote("TXT_RELEASE_NOTE");
	if(!modnote.empty()) {
		DrawText(modnote, offset, COLOR_RED);
	}

	if(Ares::bFPSCounter) {
		wchar_t buffer[0x100] = L"\0";
		swprintf_s(buffer, L"FPS: %-4u Avg: %.2f", FPSCounter::CurrentFrameRate, FPSCounter::GetAverageFrameRate());

		DrawText(buffer, offset, COLOR_WHITE);
	}

	return 0;
}

DEFINE_HOOK(78997B, sub_789960_RemoveWOLResolutionCheck, 0)
{
	return 0x789A58;
}

DEFINE_HOOK(4BA61B, DSurface_CTOR_SkipVRAM, 6)
{
	return 0x4BA623;
}

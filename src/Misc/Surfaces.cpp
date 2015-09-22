#include "Surfaces.h"

#include <Drawing.h>
#include <YRDDraw.h>
#include <WWMouseClass.h>
#include <FPSCounter.h>

#include "../Ares.h"
#include "../Utilities/Macro.h"

#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

CSFText AresSurfaces::ModNote;
std::vector<unsigned char> AresSurfaces::ShpCompression1Buffer;

DEFINE_HOOK(7C89D4, DirectDrawCreate, 6)
{
	R->Stack<DWORD>(0x4, Ares::GlobalControls::GFX_DX_Force);
	return 0;
}

DEFINE_HOOK(7B9510, WWMouseClass_DrawCursor_V1, 6)
//A_FINE_HOOK_AGAIN(7B94B2, WWMouseClass_DrawCursor_V1, 6)
{
	auto const Blitter = FileSystem::MOUSE_PAL->SelectProperBlitter(
		WWMouseClass::Instance->Image, WWMouseClass::Instance->ImageFrameIndex,
		BlitterFlags::None);

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

				_snprintf_s(fName, _TRUNCATE, "SCRN.%04u%02u%02u-%02u%02u%02u-%05u.BMP",
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

				CCFileClass *ScreenShot = GameCreate<CCFileClass>("\0");

				ScreenShot->OpenEx(fName, FileAccessMode::Write);

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
				std::unique_ptr<WORD[]> pixelData(new WORD[arrayLen]);
				WORD *pixels = pixelData.get();
				int pitch = Surface->SurfDesc->lPitch;
				for(int r = 0; r < height; ++r) {
					memcpy(pixels, reinterpret_cast<void *>(buffer), width * 2);
					pixels += width;
					buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
				}

				ScreenShot->WriteBytes(pixelData.get(), arrayLen * 2);
				ScreenShot->Close();
				GameDelete(ScreenShot);

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

	if(auto const pWarning = Ares::GetStabilityWarning()) {
		Ares::bStableNotification = true;
		DrawText(pWarning, offset, COLOR_RED);
	}

	if(!AresSurfaces::ModNote.Label) {
		AresSurfaces::ModNote = "TXT_RELEASE_NOTE";
	}

	if(!AresSurfaces::ModNote.empty()) {
		DrawText(AresSurfaces::ModNote, offset, COLOR_RED);
	}

	if(Ares::bFPSCounter) {
		wchar_t buffer[0x100];
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

DEFINE_HOOK(437CCC, BSurface_DrawSHPFrame1_Buffer, 8)
{
	REF_STACK(RectangleStruct const, bounds, STACK_OFFS(0x7C, 0x10));
	REF_STACK(unsigned char const*, pBuffer, STACK_OFFS(0x7C, 0x6C));

	auto const width = static_cast<size_t>(Math::clamp(
		bounds.Width, 0, std::numeric_limits<short>::max()));

	// buffer overrun is now not as forgiving as it was before
	auto& Buffer = AresSurfaces::ShpCompression1Buffer;
	if(Buffer.size() < width) {
		Buffer.insert(Buffer.end(), width - Buffer.size(), 0u);
	}

	pBuffer = Buffer.data();

	return 0x437CD4;
}

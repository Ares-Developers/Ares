#include "Constructs.h"

#include <ConvertClass.h>
#include <FileSystem.h>
#include <ScenarioClass.h>

bool CustomPalette::LoadFromINI(
	CCINIClass* pINI, const char* pSection, const char* pKey,
	const char* pDefault)
{
	if(pINI->ReadString(pSection, pKey, pDefault, Ares::readBuffer)) {
		if(auto const pSuffix = strstr(Ares::readBuffer, "~~~")) {
			auto const theater = ScenarioClass::Instance->Theater;
			auto const pExtension = Theater::GetTheater(theater).Extension;
			pSuffix[0] = pExtension[0];
			pSuffix[1] = pExtension[1];
			pSuffix[2] = pExtension[2];
		}

		this->Clear();

		if(auto pPal = FileSystem::AllocatePalette(Ares::readBuffer)) {
			this->Palette.reset(pPal);
			this->CreateConvert();
		}

		return this->Convert != nullptr;
	}
	return false;
}

bool CustomPalette::Load(AresStreamReader &Stm, bool RegisterForChange)
{
	this->Clear();

	bool hasPalette = false;
	auto ret = Stm.Load(this->Mode) && Stm.Load(hasPalette);

	if(ret && hasPalette) {
		this->Palette.reset(GameCreate<BytePalette>());
		ret = Stm.Load(*this->Palette);

		if(ret) {
			this->CreateConvert();
		}
	}

	return ret;
}

bool CustomPalette::Save(AresStreamWriter &Stm) const
{
	Stm.Save(this->Mode);
	Stm.Save(this->Palette != nullptr);
	if(this->Palette) {
		Stm.Save(*this->Palette);
	}
	return true;
}

void CustomPalette::Clear()
{
	this->Convert = nullptr;
	this->Palette = nullptr;
}

void CustomPalette::CreateConvert()
{
	ConvertClass* buffer = nullptr;
	if(this->Mode == PaletteMode::Temperate) {
		buffer = GameCreate<ConvertClass>(
			this->Palette.get(), FileSystem::TEMPERAT_PAL, DSurface::Primary,
			53, false);
	} else {
		buffer = GameCreate<ConvertClass>(
			this->Palette.get(), this->Palette.get(), DSurface::Alternate,
			1, false);
	}
	this->Convert.reset(buffer);
}

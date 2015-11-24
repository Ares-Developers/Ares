#include "Body.h"
#include "../../Misc/Debug.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/INIParser.h"
#include "../../Utilities/TemplateDef.h"

#include "../../Ares.CRT.h"

#include <GeneralDefinitions.h>
#include <Theater.h>
#include <TerrainTypeClass.h>
#include <HouseTypeClass.h>
#include <OverlayTypeClass.h>
#include <AnimTypeClass.h>
#include <SmudgeTypeClass.h>
#include <ScenarioClass.h>

#include <algorithm>

DEFINE_HOOK(5F9634, ObjectTypeClass_LoadFromINI, 6)
{
	GET(ObjectTypeClass *, pType, EBX);
	GET_STACK(CCINIClass *, pINI, STACK_OFFS(0x1B0, -4));

	TechnoTypeClass *pTechnoType = static_cast<TechnoTypeClass *>(pType);
	TechnoTypeExt::ExtData * pTypeData = TechnoTypeExt::ExtMap.Find(pTechnoType);

	if(pTypeData) {
		INI_EX exINI(pINI);
		pTypeData->AlternateTheaterArt.Read(exINI, pType->ID, "AlternateTheaterArt");
	}

	return 0;
}

// SHP file loading
DEFINE_HOOK(5F9070, ObjectTypeClass_Load2DArt, 0)
{
	GET(ObjectTypeClass* const, pType, ECX);

	auto const pTypeData = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType));

	char basename[MAX_PATH];

	auto const scenarioTheater = ScenarioClass::Instance->Theater;
	auto const& TheaterData = Theater::GetTheater(scenarioTheater);

	// extension object is not present if not techno type
	if(pTypeData && pTypeData->AlternateTheaterArt) {
		if(!pType->ArcticArtInUse) { // this flag is not used anywhere outside this function, so I'll just hijack it
			pType->ArcticArtInUse = true;
			_snprintf_s(basename, _TRUNCATE, "%s%s", pType->ImageFile, TheaterData.Letter);
			if(!CCINIClass::INI_Art->GetSection(basename)) {
				pType->ArcticArtInUse = false;
				_snprintf_s(basename, _TRUNCATE, "%s", pType->ImageFile);
			}
			AresCRT::strCopy(pType->ImageFile, basename);
		}
	} else if(pType->AlternateArcticArt && scenarioTheater == TheaterType::Snow && !pType->ImageAllocated) {
		if(!pType->ArcticArtInUse) {
			_snprintf_s(basename, _TRUNCATE, "%sA", pType->ImageFile);
			AresCRT::strCopy(pType->ImageFile, basename);
			pType->ArcticArtInUse = true;
		}
	} else {
		pType->ArcticArtInUse = false;
	}

	auto const pExt = (pType->Theater ? TheaterData.Extension : "SHP");
	_snprintf_s(basename, _TRUNCATE, "%s.%s", pType->ImageFile, pExt);

	if(!pType->Theater && pType->NewTheater && scenarioTheater != TheaterType::None) {
		if(isalpha(static_cast<unsigned char>(basename[0]))) {
			// evil hack to uppercase
			auto const c1 = static_cast<unsigned char>(basename[1]) & ~0x20;
			if(c1 == 'A' || c1 == 'T') {
				basename[1] = TheaterData.Letter[0];
			}
		}
	}

	if(pType->ImageAllocated && pType->Image) {
		GameDelete(pType->Image);
	}
	pType->Image = nullptr;
	pType->ImageAllocated = false;

	using namespace Helpers::Alex;
	auto const abs = pType->WhatAmI();

	// what? it's what the game does, evidently those load somewhere else
	if(!is_any_of(abs, AbstractType::SmudgeType, AbstractType::TerrainType)) {
		auto const forceShp = is_any_of(
			abs, AbstractType::OverlayType, AbstractType::AnimType);

		auto pImage = FileSystem::LoadFile(basename, forceShp);
		if(!pImage) {
			basename[1] = 'G';
			pImage = FileSystem::LoadFile(basename, forceShp);
		}

		pType->Image = static_cast<SHPStruct*>(pImage);
	}

	if(auto const pShp = pType->Image) {
		auto const& size = std::max(pShp->Width, pShp->Height);
		pType->MaxDimension = std::max(size, static_cast<short>(8));
	}

	return 0x5F92C3;
}

DEFINE_HOOK(5F96B0, ObjectTypeClass_TheaterSpecificID, 6)
{
	GET(char *, basename, ECX);
	GET(TheaterType, Theater, EDX);

	if(Theater != TheaterType::None) {
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if(isalpha(static_cast<unsigned char>(c0))) {
			if(c1 == 'A' || c1 == 'T') {
				basename[1] = Theater::GetTheater(Theater).Letter[0];
			}
		}
	}
	return 0x5F9702;
}

#include "Body.h"
#include "../../Misc/Debug.h"

#include "../../Ares.CRT.h"

#include <GeneralDefinitions.h>
#include <Theater.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>
#include <AnimTypeClass.h>
#include <SmudgeTypeClass.h>
#include <ScenarioClass.h>

#include <algorithm>

DEFINE_HOOK(5F9634, ObjectTypeClass_LoadFromINI, 6)
{
	GET(ObjectTypeClass *, pType, EBX);
	GET_STACK(CCINIClass *, pINI, STACK_OFFS(0x1B0, -4));

	TechnoTypeClass *pTechnoType = reinterpret_cast<TechnoTypeClass *>(pType);
	TechnoTypeExt::ExtData * pTypeData = TechnoTypeExt::ExtMap.Find(pTechnoType);

	if(pTypeData) {
		pTypeData->AlternateTheaterArt = pINI->ReadBool(pType->ID, "AlternateTheaterArt", pTypeData->AlternateTheaterArt);
	}

	return 0;
}

// SHP file loading
DEFINE_HOOK(5F9070, ObjectTypeClass_Load2DArt, 0)
{
	GET(ObjectTypeClass *, pType, ECX);

	TechnoTypeClass *pTechnoType = reinterpret_cast<TechnoTypeClass *>(pType);
	TechnoTypeExt::ExtData * pTypeData = TechnoTypeExt::ExtMap.Find(pTechnoType);

	char basename[256];

	int scenarioTheater = ScenarioClass::Instance->Theater;
	Theater *pTheaterData = &Theater::Array[scenarioTheater];

	if(pTypeData && pTypeData->AlternateTheaterArt) {
		if(!pType->ArcticArtInUse) { // this flag is not used anywhere outside this function, so I'll just hijack it
			pType->ArcticArtInUse = true;
			_snprintf_s(basename, 255, "%s%s", pType->ImageFile, pTheaterData->Letter);
			if(!CCINIClass::INI_Art->GetSection(basename)) {
				pType->ArcticArtInUse = false;
				_snprintf_s(basename, 255, "%s", pType->ImageFile);
			}
			AresCRT::strCopy(pType->ImageFile, basename);
		}
	} else if(pType->AlternateArcticArt && scenarioTheater == TheaterType::Snow && !pType->ImageIsOutdated) { //outdated? you think I know what it means? hahahaha
		if(!pType->ArcticArtInUse) {
			_snprintf_s(basename, 255, "%sA", pType->ImageFile);
			AresCRT::strCopy(pType->ImageFile, basename);
			pType->ArcticArtInUse = true;
		}
	} else {
		AresCRT::strCopy(basename, pType->ImageFile, 0x1A);
		pType->ArcticArtInUse = false;
	}

	_snprintf_s(basename, 255, "%s.%s", pType->ImageFile, (pType->Theater ? pTheaterData->Extension : "SHP"));

	if(!pType->Theater && pType->NewTheater && scenarioTheater != -1) {
		unsigned char c0 = basename[0];
		unsigned char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if(isalpha(c0)) {
			if(c1 == 'A' || c1 == 'T') {
				basename[1] = pTheaterData->Letter[0];
			}
		}
	}

	if(pType->ImageIsOutdated) {
		if(pType->Image) {
			GAME_DEALLOC(pType->Image);
		}
	}
	pType->Image = NULL;
	pType->ImageIsOutdated = false;

	switch(pType->WhatAmI()) {
		case SmudgeTypeClass::AbsID:
		case TerrainTypeClass::AbsID:
		// what? it's what the game does, evidently those load somewhere else
		break;

		case OverlayTypeClass::AbsID:
		case AnimTypeClass::AbsID:
			pType->Image = reinterpret_cast<SHPStruct *>(FileSystem::LoadFile(basename, true));
			if(!pType->Image) {
				basename[1] = 'G';
				pType->Image = reinterpret_cast<SHPStruct *>(FileSystem::LoadFile(basename, true));
			}
			break;

		default:
			pType->Image = reinterpret_cast<SHPStruct *>(FileSystem::LoadFile(basename, false)); // false? how would I know? it works for wastewood
			if(!pType->Image) {
				basename[1] = 'G';
				pType->Image = reinterpret_cast<SHPStruct *>(FileSystem::LoadFile(basename, true));
			}
			break;
	}

	if(SHPStruct *SHP = pType->Image) {
		pType->MaxDimension = std::max<short>(std::max<short>(SHP->Width, SHP->Height), 8);
	}

	return 0x5F92C3;
}

DEFINE_HOOK(5F96B0, ObjectTypeClass_TheaterSpecificID, 6)
{
	GET(char *, basename, ECX);
	GET(signed int, idxTheater, EDX);

	if(idxTheater != -1) {
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if(isalpha((unsigned char)c0)) {
			if(c1 == 'A' || c1 == 'T') {
				basename[1] = Theater::Array[idxTheater].Letter[0];
			}
		}
	}
	return 0x5F9702;
}

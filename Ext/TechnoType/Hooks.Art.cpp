#include "Body.h"
#include "../../Misc/Debug.h"

#include <GeneralDefinitions.h>
#include <Theater.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>
#include <AnimTypeClass.h>
#include <SmudgeTypeClass.h>
#include <ScenarioClass.h>

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

	if(pTypeData) {
		if(pTypeData->AlternateTheaterArt) {
			if(!pType->ArcticArtInUse) { // this flag is not used anywhere outside this function, so I'll just hijack it
				pType->ArcticArtInUse = true;
				_snprintf(basename, 256, "%s%s", pType->ImageFile, pTheaterData->Letter);
				if(!CCINIClass::INI_Art->GetSection(basename)) {
					pType->ArcticArtInUse = false;
					_snprintf(basename, 256, "%s", pType->ImageFile);
				}
				strncpy(pType->ImageFile, basename, 0x19);
			}
		}
	} else if(pType->AlternateArcticArt && scenarioTheater == th_Snow && !pType->IsImageAllocated) {
		if(!pType->ArcticArtInUse) {
			_snprintf(basename, 256, "%sA", pType->ImageFile);
			strncpy(pType->ImageFile, basename, 0x19);
			pType->ArcticArtInUse = true;
		}
	} else {
		strncpy(basename, pType->ImageFile, 0x1A);
		basename[0x19] = 0;
		pType->ArcticArtInUse = false;
	}

	_snprintf(basename, 256, "%s.%s", pType->ImageFile, (pType->Theater ? pTheaterData->Extension : "SHP"));

	if(!pType->Theater && pType->NewTheater && scenarioTheater != -1) {
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if(isalpha(c0)) {
			if(c1 == 'A' || c1 == 'T') {
				basename[1] = pTheaterData->Letter[0];
			}
		}
	}

	if(pType->IsImageAllocated) {
		if(pType->Image) {
			GAME_DEALLOC(pType->Image);
		}
	}
	pType->Image = NULL;
	pType->IsImageAllocated = false;

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
		pType->MaxDimension = max(max(SHP->Width, SHP->Height), 8);
	}

	return 0x5F92C3;
}

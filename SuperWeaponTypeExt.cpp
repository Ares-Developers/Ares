#include <YRPP.h>
#include "SuperWeaponTypeExt.h"

EXT_P_DECLARE(SuperWeaponTypeClass);

DynamicVectorClass<const char *> SuperWeaponTypeClassExt::CustomSWTypes;

EXT_CTOR(SuperWeaponTypeClass)
{
	if(CustomSWTypes.get_Count() == 0)
	{
		CustomSWTypes.AddItem("Animation");
		CustomSWTypes.AddItem("SonarPulse");
	}

	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
		pData->SpyPlane_Count = 1;
		pData->SpyPlane_Mission = mission_AttackAgain;

		pData->Nuke_Sound = RulesClass::Global()->get_DigSound();

		pData->EVA_Ready = -1;
		pData->EVA_Activated = -1;
		pData->EVA_Detected = -1;

		pData->Anim_Type = NULL;
		pData->Anim_ExtraZ = 0;
		
		pData->Sonar_Range = 0;
		pData->Sonar_Anim = NULL;
		pData->Sonar_Sound = -1;

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(SuperWeaponTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(SuperWeaponTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_SAVE(SuperWeaponTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_LOAD_INI(SuperWeaponTypeClass)
{
	const char * section = pThis->get_ID();
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	char buffer[256];

	pData->SpyPlane_Count = pINI->ReadInteger(section, "SpyPlane.Count", pData->SpyPlane_Count);

	PARSE_AIRCRAFT_IDX("SpyPlane.Type", pData->SpyPlane_TypeIndex);

	if(pINI->ReadString(section, "SpyPlane.Mission", MissionClass::FindName(&pData->SpyPlane_Mission), buffer, 256) > 0)
	{
		pData->SpyPlane_Mission = MissionClass::FindIndex(buffer);
	}

	PARSE_SND("Nuke.Sound", pData->Nuke_Sound);
	PARSE_EVA("EVA.Ready", pData->EVA_Ready);
	PARSE_EVA("EVA.Activated", pData->EVA_Activated);
	PARSE_EVA("EVA.Detected", pData->EVA_Detected);

	int customType;
	if(pINI->ReadString(section, "Type", "", buffer, 256) > 0)
	{
		for(int i = 0; i < CustomSWTypes.get_Count(); ++i)
		{
			if(!strcmp(buffer, CustomSWTypes.GetItem(i)))
			{
				customType = FIRST_SW_TYPE + i;
				pThis->set_ArrayIndex(customType);
				break;
			}
		}
	}

	switch(customType)
	{
		case SW_ANIMATION:
			PARSE_ANIM("Animation.Type", pData->Anim_Type);
			pData->Anim_ExtraZ = pINI->ReadInteger(section, "Animation.Height", pData->Anim_ExtraZ);
			break;
		case SW_SONARPULSE:
			PARSE_ANIM("SonarPulse.Animation", pData->Sonar_Anim);
			pData->Sonar_Range = pINI->ReadInteger(section, "SonarPulse.Range", pData->Sonar_Range);
			PARSE_SND("SonarPulse.Sound", pData->Sonar_Sound);
			break;
	}

}

// 6CD67A, 5
// decouple SpyPlane from SPYP
EXPORT_FUNC(SuperClass_Launch_SpyPlane_FindType)
{
	GET(SuperClass *, Super, EBX);
	SuperWeaponTypeClass *pThis = Super->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];

	R->set_EAX(pData->SpyPlane_TypeIndex);
	return 0x6CD684;
}

// 6CD6A6, 6
// decouple SpyPlane from allied paradrop counts
EXPORT_FUNC(SuperClass_Launch_SpyPlane_Fire)
{
	GET(SuperClass *, Super, EBX);
	GET(CellClass *,TargetCell, EDI);
	SuperWeaponTypeClass *pThis = Super->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];
	
	Super->get_Owner()->SendSpyPlanes(
		pData->SpyPlane_TypeIndex, pData->SpyPlane_Count, pData->SpyPlane_Mission, TargetCell, NULL);

	return 0x6CD6E9;
}

// 6CDDE3, 6
// decouple nuke siren from DigSound
EXPORT_FUNC(SuperClass_Launch_Nuke_Sound)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];
	
	R->set_ECX(pData->Nuke_Sound);

	return 0x6CDDE9;
}

// 6CBDD7, 6
// custom EVA Announce
EXPORT_FUNC(SuperClass_AnnounceReady)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];
	
	RET_UNLESS(pData->EVA_Ready != -1);

	VoxClass::PlayIndex(pData->EVA_Ready);

	return 0x6CBE68;
}

bool _stdcall SuperWeaponTypeClassExt::SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords)
{
	int TypeIdx = pThis->get_Type()->get_ArrayIndex();
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);

	switch(TypeIdx - FIRST_SW_TYPE)
	{
		case SW_ANIMATION:
			return SuperWeaponTypeClassExt::SuperClass_Launch_Animation(pThis, pCoords);
		case SW_SONARPULSE:
			return SuperWeaponTypeClassExt::SuperClass_Launch_SonarPulse(pThis, pCoords);
		
	}
	return 1;
}

bool SuperWeaponTypeClassExt::SuperClass_Launch_Animation(SuperClass* pThis, CellStruct* pCoords)
{
	SuperWeaponTypeClass *pType = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pType));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pType];

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	coords.Z += pData->Anim_ExtraZ;
	new AnimClass(pData->Anim_Type, coords);
	return 1;
}

bool SuperWeaponTypeClassExt::SuperClass_Launch_SonarPulse(SuperClass* pThis, CellStruct* pCoords)
{
	SuperWeaponTypeClass *pType = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pType));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pType];

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->Sonar_Anim)
	{
		new AnimClass(pData->Sonar_Anim, coords);
	}
	
	if(pData->Sonar_Sound != -1)
	{
		VocClass::PlayAt(pData->Sonar_Sound, &coords);
	}

	int countCells = CellSpread::NumCells(pData->Sonar_Range);
	for(int i = 0; i < countCells; ++i)
	{
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += *pCoords;
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
		for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject())
		{
			if(!(curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO))
			{
				continue;
			}
			TechnoClass *curT = (TechnoClass *)curObj;
			if(curT->get_CloakingStage())
			{
				curT->Uncloak(1);
			}
		}
	}

	return 1;
}


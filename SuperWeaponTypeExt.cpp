#include <YRPP.h>
#include "SuperWeaponTypeExt.h"
#include "Actions.h"
#include "Ares.h"

EXT_P_DECLARE(SuperWeaponTypeClass);

DynamicVectorClass<const char *> SuperWeaponTypeClassExt::CustomSWTypes;
SuperWeaponTypeClass *SuperWeaponTypeClassExt::CurrentSWType = NULL;

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

		pData->SW_Initialized = 0;

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
	if(!pData->SW_Initialized)
	{
		pData->Initialize();
	}

	char buffer[256];

	pData->SpyPlane_Count = pINI->ReadInteger(section, "SpyPlane.Count", pData->SpyPlane_Count);

	PARSE_AIRCRAFT_IDX("SpyPlane.Type", pData->SpyPlane_TypeIndex);

	if(pINI->ReadString(section, "SpyPlane.Mission", MissionClass::FindName(&pData->SpyPlane_Mission), buffer, 256) > 0)
	{
		pData->SpyPlane_Mission = MissionClass::FindIndex(buffer);
	}

	PARSE_SND("Nuke.Sound", pData->Nuke_Siren);
	PARSE_EVA("EVA.Ready", pData->EVA_Ready);
	PARSE_EVA("EVA.Activated", pData->EVA_Activated);
	PARSE_EVA("EVA.Detected", pData->EVA_Detected);

	int customType;

	if(pINI->ReadString(section, "Action", "", buffer, 256) > 0 && !strcmp(buffer, "Custom"))
	{
		pThis->set_Action(SW_YES_CURSOR);
		if(pINI->ReadString(section, "Type", "", buffer, 256) > 0)
		{
			for(int i = 0; i < CustomSWTypes.get_Count(); ++i)
			{
				if(!strcmp(buffer, CustomSWTypes.GetItem(i)))
				{
					customType = FIRST_SW_TYPE + i;
					pThis->set_Type(customType);
					break;
				}
			}
		}
	}

#define READ_CURSOR(key, var) \
	READCURSOR(key, var, Frame); \
	READCURSOR(key, var, Count); \
	READCURSOR(key, var, Interval); \
	READCURSOR(key, var, MiniFrame); \
	READCURSOR(key, var, MiniCount); \

#define READCURSOR(key, var, str) \
	var.str = pINI->ReadInteger(section, key "." # str, var.str);

	READ_CURSOR("Cursor", pData->SW_Cursor);
	READ_CURSOR("NoCursor", pData->SW_NoCursor);

	pData->SW_FireToShroud = pINI->ReadBool(section, "Super.FireIntoShroud", pData->SW_FireToShroud);

	MouseCursor *Cursor = &pData->SW_Cursor;

	if(pINI->ReadString(section, "Cursor.HotSpot", "", buffer, 256) > 0)
	{
		char *hotx = strtok(buffer, ",");
		if(!strcmp(hotx, "Left")) Cursor->HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) Cursor->HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) Cursor->HotX = hotspx_right;
		char *hoty = strtok(NULL, ",");
		if(!strcmp(hoty, "Top")) Cursor->HotY = hotspy_top;
		else if(!strcmp(hoty, "Middle")) Cursor->HotY = hotspy_middle;
		else if(!strcmp(hoty, "Bottom")) Cursor->HotY = hotspy_bottom;
	}

	Cursor = &pData->SW_NoCursor;
	if(pINI->ReadString(section, "NoCursor.HotSpot", "", buffer, 256) > 0)
	{
		char *hotx = strtok(buffer, ",");
		if(!strcmp(hotx, "Left")) Cursor->HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) Cursor->HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) Cursor->HotX = hotspx_right;
		char *hoty = strtok(NULL, ",");
		if(!strcmp(hoty, "Top")) Cursor->HotY = hotspy_top;
		else if(!strcmp(hoty, "Middle")) Cursor->HotY = hotspy_middle;
		else if(!strcmp(hoty, "Bottom")) Cursor->HotY = hotspy_bottom;
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
			pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);
			PARSE_SND("SonarPulse.Sound", pData->Sonar_Sound);
			break;
	}
}

void SuperWeaponTypeClassExt::SuperWeaponTypeClassData::Initialize()
{
	this->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
	this->SpyPlane_Count = 1;
	this->SpyPlane_Mission = mission_AttackAgain;

	this->Nuke_Siren = RulesClass::Global()->get_DigSound();

	this->EVA_Ready = -1;
	this->EVA_Activated = -1;
	this->EVA_Detected = -1;

	this->Anim_Type = NULL;
	this->Anim_ExtraZ = 0;
	
	this->Sonar_Range = 0;
	this->Sonar_Anim = NULL;
	this->Sonar_Sound = -1;
	this->Sonar_Delay = 15;

	this->SW_FireToShroud = 1;

	this->SW_Cursor.HotX = hotspx_center;
	this->SW_Cursor.HotY = hotspy_middle;

	this->SW_Initialized = 1;
}

bool SuperWeaponTypeClassExt::CanFireAt(SuperWeaponTypeClass *pThis, CellStruct *pCoords)
{
	if(pThis->get_Type() != SW_SONARPULSE)
	{
		return 1;
	}

	CellClass *pCell = MapClass::Global()->GetCellAt(pCoords);

	if(pCell && pCell->get_LandType() == lt_Water)
	{
		return 1;
	}
	return 0;
}


// 6CEF84, 7
EXPORT_FUNC(SuperWeaponTypeClass_GetCursorOverObject)
{
	GET(SuperWeaponTypeClass *, pThis, ECX);
//	int TypeIdx = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis) && pThis->get_Action() >= 0x7E);
	SuperWeaponTypeClassExt::CurrentSWType = pThis;

	CellStruct *pCoords = (CellStruct *)R->get_StackVar32(0x0C);

	R->set_EAX( SuperWeaponTypeClassExt::CanFireAt(pThis, pCoords)
		? SW_YES_CURSOR
		: SW_NO_CURSOR);

	return 0x6CEFD9;
}

// 6AAF92, 6
EXPORT_FUNC(SidebarClass_ProcessCameoClick)
{
	DWORD idx = R->get_ESI();
	SuperWeaponTypeClass *pThis = SuperWeaponTypeClass::Array->GetItem(idx);
//	int TypeIdx = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::CurrentSWType = pThis;
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];

//	Actions::Set(&pData->SW_Cursor);

	return 0;
}

// 4AB35A, 6
// custom SW Cursor
EXPORT_FUNC(DisplayClass_SetAction)
{
	int Action = R->get_EAX();
	RET_UNLESS(Action >= SW_NO_CURSOR);
	GET(CellStruct *, pCoords, EBX);
	DWORD dwUnk = R->get_StackVar32(0x34);
	DWORD pThis = R->get_ESI();
	bool Shrouded = R->get_StackVar8(0x28);

	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = 
		SuperWeaponTypeClassExt::Ext_p[SuperWeaponTypeClassExt::CurrentSWType];

	if(Shrouded && !pData->SW_FireToShroud)
	{
		Action = SW_NO_CURSOR;
	}

	Actions::Set(Action == SW_NO_CURSOR ? &pData->SW_NoCursor : &pData->SW_Cursor);

//	Actions::Set(Action == SW_NO_CURSOR ? &pData->SW_NoCursor : &pData->SW_Cursor);

	PUSH_VAR32(dwUnk);
	PUSH_VAR32(Action);
	SET_REG32(ECX, pThis);
	CALL_VT(0x48);
	return 0x4AB78F;
}

// 653B3A, 5
EXPORT_FUNC(RadarClass_RTacticalClass_MoreVoodoo)
{
	RET_UNLESS(Unsorted::CurrentSWType != -1);
	SuperWeaponTypeClass *pSW = SuperWeaponTypeClass::Array->GetItem(Unsorted::CurrentSWType);
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pSW];
	Actions::Set(&pData->SW_Cursor);
	R->set_ESI(pSW->get_Action());
	return 0x653CA3;
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
EXPORT_FUNC(SuperClass_Launch_Nuke_Siren)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];

	R->set_ECX(pData->Nuke_Siren);

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
	int TypeIdx = pThis->get_Type()->get_Type();
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);

	switch(TypeIdx)
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

	Unsorted::CurrentSWType = -1;
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
			if(curT->get_CloakState())
			{
				curT->Uncloak(1);
				curT->set_Sensed(1);
				//curT->set_CloakingSpeed(1);
				//curT->get_CloakTimer()->StartTime = Unsorted::CurrentFrame + pData->Sonar_Delay;
			}
		}
	}

	Unsorted::CurrentSWType = -1;
	return 1;
}

// 6CEE96, 5
EXPORT_FUNC(SuperWeaponTypeClass_GetTypeIndex)
{
	GET(const char *, TypeStr, EDI);
	for(int i = 0; i < SuperWeaponTypeClassExt::CustomSWTypes.get_Count(); ++i)
	{
		if(!strcmp(TypeStr, SuperWeaponTypeClassExt::CustomSWTypes.GetItem(i)))
		{
			R->set_ESI(FIRST_SW_TYPE + i);
			return 0x6CEE9C;
		}
	}
	return 0;
}

// 4AC20C, 7
// translates SW click to type
EXPORT_FUNC(DisplayClass_LMBUp)
{
	int Action = R->get_StackVar32(0x9C);
	if(Action < SW_NO_CURSOR)
	{
		int idx = (DWORD)SuperWeaponTypeClass::FindFirstOfAction(Action);
		R->set_EAX(idx);
		return idx ? 0x4AC21C : 0x4AC294;
	}
	else if(Action == SW_NO_CURSOR)
	{
		R->set_EAX(0);
		return 0x4AC294;
	}

	R->set_EAX((DWORD)SuperWeaponTypeClassExt::CurrentSWType);

	return 0x4AC21C;
}

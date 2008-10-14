#include "SuperWeaponTypeExt.h"

EXT_P_DEFINE(SuperWeaponTypeClass);

SuperWeaponTypeClass *SuperWeaponTypeClassExt::CurrentSWType = NULL;
DynamicVectorClass<NewSWType *> NewSWType::Array;

EXT_CTOR(SuperWeaponTypeClass)
{
	if(!NewSWType::Array.get_Count())
	{
		NewSWType::Init();
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

		SWIZZLE(Ext_p[pThis]->SW_Anim);
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

	if(pINI->ReadString(section, "Action", "", buffer, 256) > 0 && !strcmp(buffer, "Custom"))
	{
		pThis->set_Action(SW_YES_CURSOR);
	}

	if(pINI->ReadString(section, "Type", "", buffer, 256) > 0)
	{
		int customType = NewSWType::FindIndex(buffer);
		if(customType > -1)
		{
			pThis->set_Type(customType);
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
	pData->SW_AutoFire = pINI->ReadBool(section, "Super.AutoFire", pData->SW_AutoFire);

	pData->Money_Amount = pINI->ReadInteger(section, "Money.Amount", pData->Money_Amount);

	PARSE_ANIM("SW.Animation", pData->SW_Anim);
	pData->SW_AnimHeight = pINI->ReadInteger(section, "SW.AnimationHeight", pData->SW_AnimHeight);

	PARSE_SND("SW.Sound", pData->SW_Sound);

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

	int Type = pThis->get_Type() - FIRST_SW_TYPE;
	if(Type >= 0 && Type < NewSWType::Array.get_Count() )
	{
		NewSWType *swt = NewSWType::GetNthItem(pThis->get_Type());
		swt->LoadFromINI(pData, pThis, pINI);
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

	this->SW_Anim = NULL;
	this->SW_AnimHeight = 0;
	this->SW_Sound = -1;
	
	this->Sonar_Range = 0;
	this->Sonar_Delay = 15;

	this->Money_Amount = 0;

	this->SW_FireToShroud = true;
//	this->SW_AutoFire = false;

	this->SW_Cursor.Frame = 53; // Attack 
	this->SW_Cursor.Count = 5;
	this->SW_Cursor.Interval = 5; // test?
	this->SW_Cursor.MiniFrame = 52;
	this->SW_Cursor.MiniCount = 1;
	this->SW_Cursor.HotX = hotspx_center;
	this->SW_Cursor.HotY = hotspy_middle;

	this->SW_NoCursor.Frame = 0;
	this->SW_NoCursor.Count = 1;
	this->SW_NoCursor.Interval = 5;
	this->SW_NoCursor.MiniFrame = 1;
	this->SW_NoCursor.MiniCount = 1;
	this->SW_NoCursor.HotX = hotspx_center;
	this->SW_NoCursor.HotY = hotspy_middle;

	this->SW_Initialized = true;
}

// 6CEF84, 7
EXPORT_FUNC(SuperWeaponTypeClass_GetCursorOverObject)
{
	//FUCK THIS, no macros in my code xD -pd
	SuperWeaponTypeClass* pThis = (SuperWeaponTypeClass*)R->get_ECX();

	if(SuperWeaponTypeClassExt::Ext_p.find(pThis) !=
		SuperWeaponTypeClassExt::Ext_p.end())
	{
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData =
			SuperWeaponTypeClassExt::Ext_p[pThis];

		if(pThis->get_Action() >= 0x7E)
		{
			SuperWeaponTypeClassExt::CurrentSWType = pThis;

			CellStruct* pMapCoords = (CellStruct*)R->get_StackVar32(0x0C);

			int Action = SW_YES_CURSOR;
			
			if(!pData->SW_FireToShroud)
			{
				CellClass* pCell = MapClass::Global()->GetCellAt(pMapCoords);
				CoordStruct Crd;

				if(MapClass::Global()->IsLocationShrouded(pCell->GetCoords(&Crd)))
					Action = SW_NO_CURSOR;
			}

			if(pThis->get_Type() >= FIRST_SW_TYPE && !NewSWType::GetNthItem(pThis->get_Type())->CanFireAt(pMapCoords))
			{
				Action = SW_NO_CURSOR;
			}

			R->set_EAX(Action);

			Actions::Set(Action == SW_YES_CURSOR ? &pData->SW_Cursor : &pData->SW_NoCursor);
			return 0x6CEFD9;
		}
	}
	return 0;
}

/*
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
*/


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

bool _stdcall SuperWeaponTypeClassExt::SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords)
{
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis->get_Type()];
	if(pData->EVA_Activated != -1)
	{
		VoxClass::PlayIndex(pData->EVA_Activated);
	}

	if(pData->Money_Amount > 0)
	{
		DEBUGLOG("House %d gets %d credits\n", pThis->get_Owner()->get_ArrayIndex(), pData->Money_Amount);
		pThis->get_Owner()->GiveMoney(pData->Money_Amount);
	}
	else
	{
		DEBUGLOG("House %d loses %d credits\n", pThis->get_Owner()->get_ArrayIndex(), -pData->Money_Amount);
		pThis->get_Owner()->TakeMoney(-pData->Money_Amount);
	}

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->SW_Anim)
	{
		coords.Z += pData->SW_AnimHeight;
		new AnimClass(pData->SW_Anim, &coords);
	}

	if(pData->SW_Sound != -1)
	{
		VocClass::PlayAt(pData->SW_Sound, &coords, NULL);
	}

	int TypeIdx = pThis->get_Type()->get_Type();
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);
	return NewSWType::GetNthItem(TypeIdx)->Launch(pThis, pCoords);
}

// 6CEE96, 5
EXPORT_FUNC(SuperWeaponTypeClass_GetTypeIndex)
{
	GET(const char *, TypeStr, EDI);
	int customType = NewSWType::FindIndex(TypeStr);
	if(customType > -1)
	{
		R->set_ESI(customType);
		return 0x6CEE9C;
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

// decoupling sw anims from types

// 446418, 6
EXPORT_FUNC(BuildingClass_Place1)
{
	GET(BuildingClass *, pBuild, EBP);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->get_Type()->get_SuperWeapon();
	if(swTIdx == -1)
	{
		swTIdx = pBuild->get_Type()->get_SuperWeapon2();
		if(swTIdx == -1)
		{
			return 0x446580;
		}
	}

	SuperClass *pSuper = pHouse->get_Supers()->GetItem(swTIdx);
	R->set_EAX((DWORD)pSuper);
	return 0x44643E;
}

// 44656D, 6
EXPORT_FUNC(BuildingClass_Place2)
{
	return 0x446580;
}

// 45100A, 6
EXPORT_FUNC(BuildingClass_ProcessAnims1)
{
	GET(BuildingClass *, pBuild, ESI);
	GET(HouseClass *, pHouse, EAX);
	int swTIdx = pBuild->get_Type()->get_SuperWeapon();
	if(swTIdx == -1)
	{
		swTIdx = pBuild->get_Type()->get_SuperWeapon2();
		if(swTIdx == -1)
		{
			return 0x451145;
		}
	}

	SuperClass *pSuper = pHouse->get_Supers()->GetItem(swTIdx);
	R->set_EDI((DWORD)pBuild->GetTechnoType());
	R->set_EAX((DWORD)pSuper);
	return 0x451030;
}

// 451132, 6
EXPORT_FUNC(BuildingClass_ProcessAnims2)
{
	return 0x451145;
}

// EVA_Detected
// 446937, 6
EXPORT_FUNC(BuildingClass_AnnounceSW)
{
	GET(BuildingClass *, pBuild, EBP);
	int swTIdx = pBuild->get_Type()->get_SuperWeapon();
	if(swTIdx == -1)
	{
		swTIdx = pBuild->get_Type()->get_SuperWeapon2();
		if(swTIdx == -1)
		{
			return 0x44699A;
		}
	}

	SuperWeaponTypeClass *pSW = SuperWeaponTypeClass::Array->GetItem(swTIdx);
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pSW));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pSW];
	RET_UNLESS(pData->EVA_Detected != -1);
	VoxClass::PlayIndex(pData->EVA_Detected);
	return 0x44699A;
}

// EVA_Ready
// 6CBDD7, 6
EXPORT_FUNC(SuperClass_AnnounceReady)
{
	GET(SuperWeaponTypeClass *, pThis, EAX);
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pThis));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pThis];
	RET_UNLESS(pData->EVA_Ready != -1);

	VoxClass::PlayIndex(pData->EVA_Ready);
	return 0x6CBE68;
}

// 6CC0EA, 9
EXPORT_FUNC(SuperClass_Announce)
{
	GET(SuperClass *, pThis, ESI);
	SuperWeaponTypeClass *pSW = pThis->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pSW));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pSW];
	RET_UNLESS(pData->EVA_Ready != -1);

	VoxClass::PlayIndex(pData->EVA_Ready);
	return 0x6CC17E;
}

// 50B319, 6
EXPORT_FUNC(HouseClass_UpdateSWs)
{
	GET(SuperClass *, Super, ECX);
	SuperWeaponTypeClass *pSW = Super->get_Type();
	RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pSW));
	SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pSW];
	RET_UNLESS(pData->SW_AutoFire);

//	Fire! requires networking event woodoo or fake mouse clicks, bah
	return 0;
}

// EVA_Activated is complex, will do later
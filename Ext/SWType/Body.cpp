#include "Body.h"
#include "..\..\Misc\SWTypes.h"

const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x55555555;
Container<SWTypeExt> SWTypeExt::ExtMap;

SuperWeaponTypeClass *SWTypeExt::CurrentSWType = NULL;

void SWTypeExt::ExtData::InitializeConstants(SuperWeaponTypeClass *pThis)
{
	this->SpyPlane_Count = 1;
	this->SpyPlane_Mission = mission_AttackAgain;

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

	this->_Initialized = is_Constanted;
}

void SWTypeExt::ExtData::InitializeRuled(SuperWeaponTypeClass *pThis)
{
	this->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
	this->Nuke_Siren = RulesClass::Global()->DigSound;
	this->_Initialized = is_Ruled;
}

void SWTypeExt::ExtData::LoadFromINI(SuperWeaponTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();
//	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis);
	if(!pINI->GetSection(section)) {
		return;
	}

	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	char buffer[256];

	this->SpyPlane_Count = pINI->ReadInteger(section, "SpyPlane.Count", this->SpyPlane_Count);

	PARSE_AIRCRAFT_IDX("SpyPlane.Type", this->SpyPlane_TypeIndex);

	if(pINI->ReadString(section, "SpyPlane.Mission", "", buffer, 256)) {
		this->SpyPlane_Mission = MissionClass::FindIndex(buffer);
	}

	PARSE_SND("Nuke.Sound", this->Nuke_Siren);
	PARSE_EVA("EVA.Ready", this->EVA_Ready);
	PARSE_EVA("EVA.Activated", this->EVA_Activated);
	PARSE_EVA("EVA.Detected", this->EVA_Detected);

	if(pINI->ReadString(section, "Action", "", buffer, 256) && !strcmp(buffer, "Custom")) {
		pThis->set_Action(SW_YES_CURSOR);
	}

	if(pINI->ReadString(section, "Type", "", buffer, 256)) {
		int customType = NewSWType::FindIndex(buffer);
		if(customType > -1) {
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

	READ_CURSOR("Cursor", this->SW_Cursor);
	READ_CURSOR("NoCursor", this->SW_NoCursor);

	this->SW_FireToShroud = pINI->ReadBool(section, "Super.FireIntoShroud", this->SW_FireToShroud);
	this->SW_AutoFire = pINI->ReadBool(section, "Super.AutoFire", this->SW_AutoFire);

	this->Money_Amount = pINI->ReadInteger(section, "Money.Amount", this->Money_Amount);

	PARSE_ANIM("SW.Animation", this->SW_Anim);
	this->SW_AnimHeight = pINI->ReadInteger(section, "SW.AnimationHeight", this->SW_AnimHeight);

	PARSE_SND("SW.Sound", this->SW_Sound);

	MouseCursor *Cursor = &this->SW_Cursor;

	if(pINI->ReadString(section, "Cursor.HotSpot", "", buffer, 256)) {
		char *hotx = strtok(buffer, ",");
		if(!strcmp(hotx, "Left")) Cursor->HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) Cursor->HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) Cursor->HotX = hotspx_right;
		char *hoty = strtok(NULL, ",");
		if(!strcmp(hoty, "Top")) Cursor->HotY = hotspy_top;
		else if(!strcmp(hoty, "Middle")) Cursor->HotY = hotspy_middle;
		else if(!strcmp(hoty, "Bottom")) Cursor->HotY = hotspy_bottom;
	}

	Cursor = &this->SW_NoCursor;
	if(pINI->ReadString(section, "NoCursor.HotSpot", "", buffer, 256)) {
		char *hotx = strtok(buffer, ",");
		if(!strcmp(hotx, "Left")) Cursor->HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) Cursor->HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) Cursor->HotX = hotspx_right;
		char *hoty = strtok(NULL, ",");
		if(!strcmp(hoty, "Top")) Cursor->HotY = hotspy_top;
		else if(!strcmp(hoty, "Middle")) Cursor->HotY = hotspy_middle;
		else if(!strcmp(hoty, "Bottom")) Cursor->HotY = hotspy_bottom;
	}

	int Type = pThis->Type - FIRST_SW_TYPE;
	if(Type >= 0 && Type < NewSWType::Array.Count ) {
		NewSWType *swt = NewSWType::GetNthItem(pThis->Type);
		swt->LoadFromINI(this, pThis, pINI);
	}
}

bool _stdcall SWTypeExt::SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords)
{
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type);
	if(pData->EVA_Activated != -1) {
		VoxClass::PlayIndex(pData->EVA_Activated);
	}

	if(pData->Money_Amount > 0) {
		DEBUGLOG("House %d gets %d credits\n", pThis->Owner->ArrayIndex, pData->Money_Amount);
		pThis->Owner->GiveMoney(pData->Money_Amount);
	} else {
		DEBUGLOG("House %d loses %d credits\n", pThis->Owner->ArrayIndex, -pData->Money_Amount);
		pThis->Owner->TakeMoney(-pData->Money_Amount);
	}

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->SW_Anim) {
		coords.Z += pData->SW_AnimHeight;
		new AnimClass(pData->SW_Anim, &coords);
	}

	if(pData->SW_Sound != -1) {
		VocClass::PlayAt(pData->SW_Sound, &coords, NULL);
	}

	int TypeIdx = pThis->Type->Type;
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);
	return NewSWType::GetNthItem(TypeIdx)->Launch(pThis, pCoords);
}

// =============================
// container hooks

DEFINE_HOOK(6CE6F6, SuperWeaponTypeClass_CTOR, 5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6CEFE0, SuperWeaponTypeClass_DTOR, 8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(6CE8BE, SuperWeaponTypeClass_Load, 7)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x20C);
	GET_STACK(IStream*, pStm, 0x210);

	SWTypeExt::ExtMap.Load(pItem, pStm);
	return 0;
}

DEFINE_HOOK(6CE8EA, SuperWeaponTypeClass_Save, 3)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SWTypeExt::ExtMap.Save(pItem, pStm);
	return 0;
}

DEFINE_HOOK(6CEE43, SuperWeaponTypeClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(6CEE50, SuperWeaponTypeClass_LoadFromINI, A)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

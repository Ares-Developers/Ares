#include "Body.h"
#include "../../Misc/SWTypes.h"

const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x55555555;
Container<SWTypeExt> SWTypeExt::ExtMap;

SWTypeExt::TT *Container<SWTypeExt>::SavingObject = NULL;
IStream *Container<SWTypeExt>::SavingStream = NULL;

SuperWeaponTypeClass *SWTypeExt::CurrentSWType = NULL;

void SWTypeExt::ExtData::InitializeConstants(SuperWeaponTypeClass *pThis)
{
	if(!NewSWType::Array.Count) {
		NewSWType::Init();
	}

	MouseCursor *Cursor = &this->SW_Cursor;
	Cursor->Frame = 53; // Attack 
	Cursor->Count = 5;
	Cursor->Interval = 5; // test?
	Cursor->MiniFrame = 52;
	Cursor->MiniCount = 1;
	Cursor->HotX = hotspx_center;
	Cursor->HotY = hotspy_middle;

	Cursor = &this->SW_NoCursor;
	Cursor->Frame = 0;
	Cursor->Count = 1;
	Cursor->Interval = 5;
	Cursor->MiniFrame = 1;
	Cursor->MiniCount = 1;
	Cursor->HotX = hotspx_center;
	Cursor->HotY = hotspy_middle;

	this->_Initialized = is_Constanted;
}

void SWTypeExt::ExtData::InitializeRuled(SuperWeaponTypeClass *pThis)
{
	this->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
	this->Nuke_Siren = RulesClass::Global()->DigSound;
	this->_Initialized = is_Ruled;
}

void SWTypeExt::ExtData::LoadFromINIFile(SuperWeaponTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	this->SpyPlane_Count.Read(&exINI, section, "SpyPlane.Count");

	this->SpyPlane_TypeIndex.Read(&exINI, section, "SpyPlane.Type");

	this->SpyPlane_Mission.Read(&exINI, section, "SpyPlane.Mission");

	this->Nuke_Siren.Read(&exINI, section, "Nuke.Sound");
	this->EVA_Ready.Read(&exINI, section, "EVA.Ready");
	this->EVA_Activated.Read(&exINI, section, "EVA.Activated");
	this->EVA_Detected.Read(&exINI, section, "EVA.Detected");

	if(exINI.ReadString(section, "Action") && !strcmp(exINI.value(), "Custom")) {
		pThis->Action = SW_YES_CURSOR;
	}

	if(exINI.ReadString(section, "Type")) {
		int customType = NewSWType::FindIndex(exINI.value());
		if(customType > -1) {
			pThis->Type = customType;
		}
	}

	this->SW_FireToShroud.Read(&exINI, section, "Super.FireIntoShroud");
	this->SW_AutoFire.Read(&exINI, section, "Super.AutoFire");
	this->SW_RadarEvent.Read(&exINI, section, "Super.CreateRadarEvent");

	this->Money_Amount.Read(&exINI, section, "Money.Amount");

	this->SW_Anim.Parse(&exINI, section, "SW.Animation");
	this->SW_AnimHeight.Read(&exINI, section, "SW.AnimationHeight");

	this->SW_Sound.Read(&exINI, section, "SW.Sound");

	this->SW_Cursor.Read(&exINI, section, "Cursor");
	this->SW_NoCursor.Read(&exINI, section, "NoCursor");

	int Type = pThis->Type - FIRST_SW_TYPE;
	if(Type >= 0 && Type < NewSWType::Array.Count ) {
		NewSWType *swt = NewSWType::GetNthItem(pThis->Type);
		swt->LoadFromINI(this, pThis, pINI);
	}
}

bool _stdcall SWTypeExt::SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type);

	if(pData->EVA_Activated != -1) {
		VoxClass::PlayIndex(pData->EVA_Activated);
	}

	int Money_Amount = pData->Money_Amount;
	if(Money_Amount > 0) {
		DEBUGLOG("House %d gets %d credits\n", pThis->Owner->ArrayIndex, Money_Amount);
		pThis->Owner->GiveMoney(Money_Amount);
	} else if(Money_Amount < 0) {
		DEBUGLOG("House %d loses %d credits\n", pThis->Owner->ArrayIndex, -Money_Amount);
		pThis->Owner->TakeMoney(-Money_Amount);
	}

	CoordStruct coords;
	MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->SW_Anim != NULL) {
		coords.Z += pData->SW_AnimHeight;
		AnimClass *placeholder;
		GAME_ALLOC(AnimClass, placeholder, pData->SW_Anim, &coords);
	}

	if(pData->SW_Sound != -1) {
		VocClass::PlayAt(pData->SW_Sound, &coords, NULL);
	}

	if(pData->SW_RadarEvent) {
		RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, *pCoords);
	}

	int TypeIdx = pThis->Type->Type;
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);
	return NewSWType::GetNthItem(TypeIdx)->Launch(pThis, pCoords, IsPlayer);
}

// =============================
// load/save

void Container<SWTypeExt>::Load(SuperWeaponTypeClass *pThis, IStream *pStm) {
	SWTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->SW_Anim);
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

DEFINE_HOOK(6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, A)
DEFINE_HOOK_AGAIN(6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(SWTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8); 

	Container<SWTypeExt>::SavingObject = pItem;
	Container<SWTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(6CE8BE, SuperWeaponTypeClass_Load_Suffix, 7)
{
	SWTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6CE8EA, SuperWeaponTypeClass_Save_Suffix, 3)
{
	SWTypeExt::ExtMap.SaveStatic();
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

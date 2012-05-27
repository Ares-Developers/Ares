#include <PCX.h>

#include "Body.h"
#include "../../Misc/SWTypes.h"
#include "../House/Body.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"
#include "../../Ares.h"
#include "../../Ares.CRT.h"
#include "../../Utilities/Enums.h"

#include <WarheadTypeClass.h>
#include <MessageListClass.h>

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x55555555;
Container<SWTypeExt> SWTypeExt::ExtMap;

template<> SWTypeExt::TT *Container<SWTypeExt>::SavingObject = NULL;
template<> IStream *Container<SWTypeExt>::SavingStream = NULL;

SuperWeaponTypeClass *SWTypeExt::CurrentSWType = NULL;

SWTypeExt::ExtData::~ExtData() {
	this->ParaDrop.clear();

	for(int i=this->ParaDropPlanes.Count-1; i>=0; --i) {
		delete this->ParaDropPlanes.Items[i];
		this->ParaDropPlanes.Items[i] = NULL;
	}
};

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

	AresCRT::strCopy(this->Text_Ready, "TXT_READY", 0x20);
	AresCRT::strCopy(this->Text_Hold, "TXT_HOLD", 0x20);
	AresCRT::strCopy(this->Text_Charging, "TXT_CHARGING", 0x20);
	AresCRT::strCopy(this->Text_Active, "TXT_FIRESTORM_ON", 0x20);

	EVA_InsufficientFunds = VoxClass::FindIndex("EVA_InsufficientFunds");
}

void SWTypeExt::ExtData::InitializeRuled(SuperWeaponTypeClass *pThis)
{
}

void SWTypeExt::ExtData::LoadFromRulesFile(SuperWeaponTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	if(exINI.ReadString(section, "Action") && !_strcmpi(exINI.value(), "Custom")) {
		pThis->Action = SW_YES_CURSOR;
	}

	if(exINI.ReadString(section, "Type")) {
		int customType = NewSWType::FindIndex(exINI.value());
		if(customType > -1) {
			pThis->Type = customType;
		}
	}

	// find a NewSWType that handles this original one.
	int idxNewSWType = -1;
	if(pThis->Type < FIRST_SW_TYPE) {
		this->HandledByNewSWType = NewSWType::FindHandler(pThis->Type);
		idxNewSWType = this->HandledByNewSWType;
	} else {
		idxNewSWType = pThis->Type;
	}

	// if this is handled by a NewSWType, initialize it.
	if(idxNewSWType != -1) {
		pThis->Action = SW_YES_CURSOR;
		if(NewSWType *swt = NewSWType::GetNthItem(idxNewSWType)) {
			swt->Initialize(this, pThis);
		}
	}
	this->LastAction = pThis->Action;
}

void SWTypeExt::ExtData::LoadFromINIFile(SuperWeaponTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	// read general properties
	this->EVA_Ready.Read(&exINI, section, "EVA.Ready");
	this->EVA_Activated.Read(&exINI, section, "EVA.Activated");
	this->EVA_Detected.Read(&exINI, section, "EVA.Detected");
	this->EVA_Impatient.Read(&exINI, section, "EVA.Impatient");
	this->EVA_InsufficientFunds.Read(&exINI, section, "EVA.InsufficientFunds");

	this->SW_FireToShroud.Read(&exINI, section, "SW.FireIntoShroud");
	this->SW_AutoFire.Read(&exINI, section, "SW.AutoFire");
	this->SW_ManualFire.Read(&exINI, section, "SW.ManualFire");
	this->SW_RadarEvent.Read(&exINI, section, "SW.CreateRadarEvent");
	this->SW_ShowCameo.Read(&exINI, section, "SW.ShowCameo");
	this->SW_Unstoppable.Read(&exINI, section, "SW.Unstoppable");

	this->Money_Amount.Read(&exINI, section, "Money.Amount");
	this->Money_DrainAmount.Read(&exINI, section, "Money.DrainAmount");
	this->Money_DrainDelay.Read(&exINI, section, "Money.DrainDelay");

	this->SW_Sound.Read(&exINI, section, "SW.Sound");
	this->SW_ActivationSound.Read(&exINI, section, "SW.ActivationSound");

	this->SW_Anim.Parse(&exINI, section, "SW.Animation");
	this->SW_AnimHeight.Read(&exINI, section, "SW.AnimationHeight");

	this->SW_AnimVisibility.Read(&exINI, section, "SW.AnimationVisibility");
	this->SW_AffectsHouse.Read(&exINI, section, "SW.AffectsHouse");
	this->SW_AITargetingType.Read(&exINI, section, "SW.AITargeting");
	this->SW_AffectsTarget.Read(&exINI, section, "SW.AffectsTarget");
	this->SW_RequiresTarget.Read(&exINI, section, "SW.RequiresTarget");
	this->SW_RequiresHouse.Read(&exINI, section, "SW.RequiresHouse");

	this->SW_Deferment.Read(&exINI, section, "SW.Deferment");
	this->SW_ChargeToDrainRatio.Read(&exINI, section, "SW.ChargeToDrainRatio");

	this->SW_Cursor.Read(&exINI, section, "Cursor");
	this->SW_NoCursor.Read(&exINI, section, "NoCursor");

	this->SW_Warhead.Parse(&exINI, section, "SW.Warhead");
	this->SW_Damage.Read(&exINI, section, "SW.Damage");

	if(pINI->ReadString(section, "SW.Range", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		char* p = strtok(Ares::readBuffer, Ares::readDelims);
		if(p && *p) {
			this->SW_WidthOrRange = (float)atof(p);
			this->SW_Height = -1;

			p = strtok(NULL, Ares::readDelims);
			if(p && *p) {
				this->SW_Height = atoi(p);
			}
		}
	}

	// lighting
	this->Lighting_Enabled.Read(&exINI, section, "Light.Enabled");
	this->Lighting_Ambient.Read(&exINI, section, "Light.Ambient");
	this->Lighting_Red.Read(&exINI, section, "Light.Red");
	this->Lighting_Green.Read(&exINI, section, "Light.Green");
	this->Lighting_Blue.Read(&exINI, section, "Light.Blue");

	auto readString = [&](char* value, char* key) {
		if(pINI->ReadString(section, key, Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value, Ares::readBuffer, 0x20);
		}
	};

	// messages and their properties
	this->Message_FirerColor.Read(&exINI, section, "Message.FirerColor");
	if(pINI->ReadString(section, "Message.Color", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		this->Message_ColorScheme = ColorScheme::FindIndex(Ares::readBuffer);
		if(!this->Message_ColorScheme) {
			Debug::INIParseFailed(section, "Message.Color", Ares::readBuffer, "Expected a valid color scheme name.");
		}
	}

	readString(this->Message_Detected, "Message.Detected");
	readString(this->Message_Ready, "Message.Ready");
	readString(this->Message_Launch, "Message.Launch");
	readString(this->Message_Activate, "Message.Activate");
	readString(this->Message_Abort, "Message.Abort");
	readString(this->Message_InsufficientFunds, "Message.InsufficientFunds");

	readString(this->Text_Preparing, "Text.Preparing");
	readString(this->Text_Ready, "Text.Ready");
	readString(this->Text_Hold, "Text.Hold");
	readString(this->Text_Charging, "Text.Charging");
	readString(this->Text_Active, "Text.Active");

	this->NameReadiness_Preparing = NULL;
	this->NameReadiness_Ready = NULL;
	this->NameReadiness_Hold = NULL;
	this->NameReadiness_Charging = NULL;
	this->NameReadiness_Active = NULL;

	// the fallback is handled in the PreDependent SW's code
	if(pINI->ReadString(section, "SW.PostDependent", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->SW_PostDependent, Ares::readBuffer, 0x18);
	}

	// find a NewSWType that handles this original one.
	int idxNewSWType = ((pThis->Type < FIRST_SW_TYPE) ? this->HandledByNewSWType : pThis->Type);

	// initialize the NewSWType that handles this SWType.
	int Type = idxNewSWType - FIRST_SW_TYPE;
	if(Type >= 0 && Type < NewSWType::Array.Count) {
		pThis->Action = this->LastAction;
		NewSWType *swt = NewSWType::GetNthItem(idxNewSWType);
		swt->LoadFromINI(this, pThis, pINI);
		this->LastAction = pThis->Action;

		// whatever the user does, we take care of the stupid tags.
		// there is no need to have them not hardcoded.
		SuperWeaponFlags::Value flags = swt->Flags();
		pThis->PreClick = ((flags & SuperWeaponFlags::PreClick) != 0);
		pThis->PostClick = ((flags & SuperWeaponFlags::PostClick) != 0);
		pThis->PreDependent = -1;
	}

	this->CameoPal.LoadFromINI(pINI, pThis->ID, "SidebarPalette");

	if(pINI->ReadString(section, "SidebarPCX", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->SidebarPCX, Ares::readBuffer, 0x20);
		_strlwr_s(this->SidebarPCX, 0x20);
		if(!PCX::Instance->LoadFile(this->SidebarPCX)) {
			Debug::INIParseFailed(section, "SidebarPCX", this->SidebarPCX);
		}
	}
}

// can i see the animation of pFirer's SW?
bool SWTypeExt::ExtData::IsAnimVisible(HouseClass* pFirer) {
	SuperWeaponAffectedHouse::Value relation = GetRelation(pFirer, HouseClass::Player);
	return (this->SW_AnimVisibility & relation) == relation;
}

// does pFirer's SW affects object belonging to pHouse?
bool SWTypeExt::ExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse) {
	return IsHouseAffected(pFirer, pHouse, this->SW_AffectsHouse);
}

bool SWTypeExt::ExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, SuperWeaponAffectedHouse::Value value) {
	SuperWeaponAffectedHouse::Value relation = GetRelation(pFirer, pHouse);
	return (value & relation) == relation;
}

SuperWeaponAffectedHouse::Value SWTypeExt::ExtData::GetRelation(HouseClass* pFirer, HouseClass* pHouse) {
	// that's me!
	if(pFirer == pHouse) {
		return SuperWeaponAffectedHouse::Owner;
	}

	if(pFirer->IsAlliedWith(pHouse)) {
		// only friendly houses
		return SuperWeaponAffectedHouse::Allies;
	}

	// the bad guys
	return SuperWeaponAffectedHouse::Enemies;
}

bool SWTypeExt::ExtData::IsCellEligible(CellClass* pCell, SuperWeaponTarget::Value allowed) {
	if(allowed & SuperWeaponTarget::AllCells) {
		bool isWater = (pCell->LandType == lt_Water);
		if(isWater && !(allowed & SuperWeaponTarget::Water)) {
			// doesn't support water
			return false;
		} else if(!isWater && !(allowed & SuperWeaponTarget::Land)) {
			// doesn't support non-water
			return false;
		}
	}
	return true;
}

bool SWTypeExt::ExtData::IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget::Value allowed) {
	if(allowed & SuperWeaponTarget::AllContents) {
		if(pTechno) {
			eAbstractType abs_Techno = pTechno->WhatAmI();
			if((abs_Techno == abs_Infantry) && !(allowed & SuperWeaponTarget::Infantry)) {
				return false;
			} else if(((abs_Techno == abs_Unit) || (abs_Techno == abs_Aircraft)) && !(allowed & SuperWeaponTarget::Unit)) {
				return false;
			} else if((abs_Techno == abs_Building) && !(allowed & SuperWeaponTarget::Building)) {
				return false;
			}
		} else {
			// is the target cell allowed to be empty?
			return (allowed & SuperWeaponTarget::NoContent) != 0;
		}
	}
	return true;
}

bool SWTypeExt::ExtData::IsTechnoAffected(TechnoClass* pTechno) {
	// check land and water cells
	if(!IsCellEligible(pTechno->GetCell(), this->SW_AffectsTarget)) {
		return false;
	}

	// check for specific techno type
	if(!IsTechnoEligible(pTechno, this->SW_AffectsTarget)) {
		return false;
	}

	// no restriction
	return true;
}

bool SWTypeExt::ExtData::CanFireAt(CellStruct *pCoords) {
	if(CellClass *pCell = MapClass::Instance->GetCellAt(pCoords)) {

		// check cell type
		if(!IsCellEligible(pCell, this->SW_RequiresTarget)) {
			return false;
		}

		// check for techno type match
		TechnoClass *pTechno = generic_cast<TechnoClass*>(pCell->GetContent());
		if(pTechno && this->SW_RequiresHouse != SuperWeaponAffectedHouse::None) {
			if(!IsHouseAffected(HouseClass::Player, pTechno->Owner, this->SW_RequiresHouse)) {
				return false;
			}
		}

		if(!IsTechnoEligible(pTechno, this->SW_RequiresTarget)) {
			return false;
		}
	}

	// no restriction
	return true;
}

bool SWTypeExt::Launch(SuperClass* pThis, NewSWType* pSW, CellStruct* pCoords, byte IsPlayer) {
	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type)) {

		// launch the SW, then play sounds and animations. if the SW isn't launched
		// nothing will be played.
		if(pSW->Launch(pThis, pCoords, IsPlayer)) {
			SuperWeaponFlags::Value flags = pSW->Flags();

			if(flags & SuperWeaponFlags::PostClick) {
				// use the properties of the originally fired SW
				if(HouseExt::ExtData *pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
					if(pThis->Owner->Supers.ValidIndex(pExt->SWLastIndex)) {
						pThis = pThis->Owner->Supers.GetItem(pExt->SWLastIndex);
						pData = SWTypeExt::ExtMap.Find(pThis->Type);
					}
				}
			}

			if(!Unsorted::MuteSWLaunches && (pData->EVA_Activated != -1) && !(flags & SuperWeaponFlags::NoEVA)) {
				VoxClass::PlayIndex(pData->EVA_Activated);
			}
			
			if(!(flags & SuperWeaponFlags::NoMoney)) {
				int Money_Amount = pData->Money_Amount;
				if(Money_Amount > 0) {
					DEBUGLOG("House %d gets %d credits\n", pThis->Owner->ArrayIndex, Money_Amount);
					pThis->Owner->GiveMoney(Money_Amount);
				} else if(Money_Amount < 0) {
					DEBUGLOG("House %d loses %d credits\n", pThis->Owner->ArrayIndex, -Money_Amount);
					pThis->Owner->TakeMoney(-Money_Amount);
				}
			}

			CellClass *pTarget = MapClass::Instance->GetCellAt(pCoords);

			CoordStruct coords;
			pTarget->GetCoordsWithBridge(&coords);

			if((pData->SW_Anim.Get() != NULL) && !(flags & SuperWeaponFlags::NoAnim)) {
				coords.Z += pData->SW_AnimHeight;
				AnimClass *placeholder;
				GAME_ALLOC(AnimClass, placeholder, pData->SW_Anim, &coords);
				placeholder->Invisible = !pData->IsAnimVisible(pThis->Owner);
			}

			if((pData->SW_Sound != -1) && !(flags & SuperWeaponFlags::NoSound)) {
				VocClass::PlayAt(pData->SW_Sound, &coords, NULL);
			}

			if(pData->SW_RadarEvent.Get() && !(flags & SuperWeaponFlags::NoEvent)) {
				RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, *pCoords);
			}

			if(pData->Message_Launch && !(flags & SuperWeaponFlags::NoMessage)) {
				pData->PrintMessage(pData->Message_Launch, pThis->Owner);
			}

			// this sw has been fired. clean up.
			int idxThis = pThis->Owner->Supers.FindItemIndex(&pThis);
			if(IsPlayer && !(flags & SuperWeaponFlags::NoCleanup)) {
				// what's this? we reset the selected SW only for the player on this
				// computer, so others don't deselect it when firing simultaneously.
				// and we only do this, if this type's index is the current one, because
				// auto-firing might happen while the player still selects a target.
				// PostClick SWs do have a different type index, so they need to be
				// special cased, but they can't auto-fire anyhow.
				if(pThis->Owner == HouseClass::Player) {
					if(idxThis == Unsorted::CurrentSWType || (flags & SuperWeaponFlags::PostClick)) {
						Unsorted::CurrentSWType = -1;
					}
				}

				// do not play ready sound. this thing just got off.
				if(pData->EVA_Ready != -1) {
					VoxClass::SilenceIndex(pData->EVA_Ready);
				}
			}

			if(HouseExt::ExtData *pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
				// post-click actions. AutoFire SWs cannot support this. Consider
				// two Chronospheres auto-firing (the second may be launched manually).
				// the ChronoWarp would chose the source selected last, because it
				// would overwrite the previous (unfired) SW's index.
				if(!(flags & SuperWeaponFlags::PostClick) && !pData->SW_AutoFire) {
					pExt->SWLastIndex = idxThis;
				}
			}
	
			return true;
		}
	}

	return false;
}

NewSWType* SWTypeExt::ExtData::GetNewSWType() {
	int TypeIdx = (this->HandledByNewSWType != -1 ? this->HandledByNewSWType : this->AttachedToObject->Type);
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);

	if(NewSWType* pSW = NewSWType::GetNthItem(TypeIdx)) {
		return pSW;
	}

	return NULL;
}

void SWTypeExt::ExtData::PrintMessage(char* pMessage, HouseClass* pFirer) {
	if(!pMessage || !*pMessage) {
		return;
	}

	int color = ColorScheme::FindIndex("Gold");
	if(this->Message_FirerColor.Get()) {
		// firer color
		if(pFirer) {
			color = pFirer->ColorSchemeIndex;
		}
	} else {
		if(this->Message_ColorScheme > -1) {
			// user defined color
			color = this->Message_ColorScheme;
		} else if(HouseClass::Player) {
			// default way: the current player's color
			color = HouseClass::Player->ColorSchemeIndex;
		}
	}

	// print the message
	const wchar_t* label = StringTable::LoadStringA(pMessage);
	if(label && *label) {
		MessageListClass::Instance->PrintMessage(label, color);
	}
}

void SWTypeExt::ClearChronoAnim(SuperClass *pThis)
{
	DynamicVectorClass<SuperClass*>* pSupers = (DynamicVectorClass<SuperClass*>*)0xB0F5B8;

	if(pThis->Animation) {
		pThis->Animation->RemainingIterations = 0;
		pThis->Animation = NULL;
		int idx = pSupers->FindItemIndex(&pThis);
		if(idx != -1) {
			pSupers->RemoveItem(idx);
		}
	}

	if(pThis->unknown_bool_6C) {
		int idx = pSupers->FindItemIndex(&pThis);
		if(idx != -1) {
			pSupers->RemoveItem(idx);
		}
		pThis->unknown_bool_6C = false;
	}
}

void SWTypeExt::CreateChronoAnim(SuperClass *pThis, CoordStruct *pCoords, AnimTypeClass *pAnimType)
{
	DynamicVectorClass<SuperClass*>* pSupers = (DynamicVectorClass<SuperClass*>*)0xB0F5B8;

	ClearChronoAnim(pThis);
	
	if(pAnimType && pCoords) {
		AnimClass* pAnim = NULL;
		GAME_ALLOC(AnimClass, pAnim, pAnimType, pCoords);
		if(pAnim) {
			SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type);
			pAnim->Invisible = !pData->IsAnimVisible(pThis->Owner);
			pThis->Animation = pAnim;
			pSupers->AddItem(pThis);
		}
	}
}

bool SWTypeExt::ChangeLighting(SuperClass *pThis) {
	if(pThis) {
		return ChangeLighting(pThis->Type);
	}
	return false;
}

bool SWTypeExt::ChangeLighting(SuperWeaponTypeClass *pThis) {
	if(pThis) {
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis)) {
			return pData->ChangeLighting();
		}
	}
	return false;
}

bool SWTypeExt::ExtData::ChangeLighting() {
	if(this->Lighting_Enabled.Get()) {
		auto getValue = [](int value, int def) -> int {
			return (value < 0) ? def : value;
		};

		ScenarioClass* scen = ScenarioClass::Instance;
		scen->AmbientTarget = getValue(this->Lighting_Ambient, scen->AmbientOriginal);
		int cG = 1000 * getValue(this->Lighting_Green, scen->Green) / 100;
		int cB = 1000 * getValue(this->Lighting_Blue, scen->Blue)  / 100;
		int cR = 1000 * getValue(this->Lighting_Red, scen->Red)  / 100;
		scen->RecalcLighting(cR, cG, cB, 1);
		return true;
	}

	return false;
}

void Container<SWTypeExt>::InvalidatePointer(void *ptr) {
	AnnounceInvalidPointer(SWTypeExt::CurrentSWType, ptr);
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

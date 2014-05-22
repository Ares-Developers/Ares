#include <PCX.h>

#include "Body.h"
#include "../../Misc/SWTypes.h"
#include "../House/Body.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"
#include "../../Ares.h"
#include "../../Ares.CRT.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/TemplateDef.h"

#include <RadarEventClass.h>
#include <SuperClass.h>
#include <AnimClass.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>
#include <MessageListClass.h>
#include <Notifications.h>

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x55555555;
Container<SWTypeExt> SWTypeExt::ExtMap;

template<> SWTypeExt::TT *Container<SWTypeExt>::SavingObject = nullptr;
template<> IStream *Container<SWTypeExt>::SavingStream = nullptr;

SuperWeaponTypeClass *SWTypeExt::CurrentSWType = nullptr;

void SWTypeExt::ExtData::InitializeConstants(SuperWeaponTypeClass *pThis)
{
	NewSWType::Init();

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

	this->Text_Ready = CSFText("TXT_READY");
	this->Text_Hold = CSFText("TXT_HOLD");
	this->Text_Charging = CSFText("TXT_CHARGING");
	this->Text_Active = CSFText("TXT_FIRESTORM_ON");

	EVA_InsufficientFunds = VoxClass::FindIndex("EVA_InsufficientFunds");
	EVA_SelectTarget = VoxClass::FindIndex("EVA_SelectTarget");
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
	if(this->IsOriginalType()) {
		this->HandledByNewSWType = NewSWType::FindHandler(pThis->Type);
	}

	// if this is handled by a NewSWType, initialize it.
	if(auto pNewSWType = this->GetNewSWType()) {
		pThis->Action = SW_YES_CURSOR;
		pNewSWType->Initialize(this, pThis);
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
	this->EVA_Ready.Read(exINI, section, "EVA.Ready");
	this->EVA_Activated.Read(exINI, section, "EVA.Activated");
	this->EVA_Detected.Read(exINI, section, "EVA.Detected");
	this->EVA_Impatient.Read(exINI, section, "EVA.Impatient");
	this->EVA_InsufficientFunds.Read(exINI, section, "EVA.InsufficientFunds");
	this->EVA_SelectTarget.Read(exINI, section, "EVA.SelectTarget");

	this->SW_FireToShroud.Read(exINI, section, "SW.FireIntoShroud");
	this->SW_AutoFire.Read(exINI, section, "SW.AutoFire");
	this->SW_ManualFire.Read(exINI, section, "SW.ManualFire");
	this->SW_RadarEvent.Read(exINI, section, "SW.CreateRadarEvent");
	this->SW_ShowCameo.Read(exINI, section, "SW.ShowCameo");
	this->SW_Unstoppable.Read(exINI, section, "SW.Unstoppable");

	this->Money_Amount.Read(exINI, section, "Money.Amount");
	this->Money_DrainAmount.Read(exINI, section, "Money.DrainAmount");
	this->Money_DrainDelay.Read(exINI, section, "Money.DrainDelay");

	this->SW_Sound.Read(exINI, section, "SW.Sound");
	this->SW_ActivationSound.Read(exINI, section, "SW.ActivationSound");

	this->SW_Anim.Read(exINI, section, "SW.Animation");
	this->SW_AnimHeight.Read(exINI, section, "SW.AnimationHeight");

	this->SW_AnimVisibility.Read(exINI, section, "SW.AnimationVisibility");
	this->SW_AffectsHouse.Read(exINI, section, "SW.AffectsHouse");
	this->SW_AITargetingType.Read(exINI, section, "SW.AITargeting");
	this->SW_AffectsTarget.Read(exINI, section, "SW.AffectsTarget");
	this->SW_RequiresTarget.Read(exINI, section, "SW.RequiresTarget");
	this->SW_RequiresHouse.Read(exINI, section, "SW.RequiresHouse");

	this->SW_Deferment.Read(exINI, section, "SW.Deferment");
	this->SW_ChargeToDrainRatio.Read(exINI, section, "SW.ChargeToDrainRatio");

	this->SW_Cursor.Read(exINI, section, "Cursor");
	this->SW_NoCursor.Read(exINI, section, "NoCursor");

	this->SW_Warhead.Read(exINI, section, "SW.Warhead");
	this->SW_Damage.Read(exINI, section, "SW.Damage");

	if(pINI->ReadString(section, "SW.Range", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		char* context = nullptr;
		char* p = strtok_s(Ares::readBuffer, Ares::readDelims, &context);
		if(p && *p) {
			this->SW_Range.WidthOrRange = (float)atof(p);
			this->SW_Range.Height = -1;

			p = strtok_s(nullptr, Ares::readDelims, &context);
			if(p && *p) {
				this->SW_Range.Height = atoi(p);
			}
		}
	}

	// lighting
	this->Lighting_Enabled.Read(exINI, section, "Light.Enabled");
	this->Lighting_Ambient.Read(exINI, section, "Light.Ambient");
	this->Lighting_Red.Read(exINI, section, "Light.Red");
	this->Lighting_Green.Read(exINI, section, "Light.Green");
	this->Lighting_Blue.Read(exINI, section, "Light.Blue");

	// messages and their properties
	this->Message_FirerColor.Read(exINI, section, "Message.FirerColor");
	if(pINI->ReadString(section, "Message.Color", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		this->Message_ColorScheme = ColorScheme::FindIndex(Ares::readBuffer);
		if(this->Message_ColorScheme < 0) {
			Debug::INIParseFailed(section, "Message.Color", Ares::readBuffer, "Expected a valid color scheme name.");
		}
	}

	this->Message_Detected.Read(exINI, section, "Message.Detected");
	this->Message_Ready.Read(exINI, section, "Message.Ready");
	this->Message_Launch.Read(exINI, section, "Message.Launch");
	this->Message_Activate.Read(exINI, section, "Message.Activate");
	this->Message_Abort.Read(exINI, section, "Message.Abort");
	this->Message_InsufficientFunds.Read(exINI, section, "Message.InsufficientFunds");

	this->Text_Preparing.Read(exINI, section, "Text.Preparing");
	this->Text_Ready.Read(exINI, section, "Text.Ready");
	this->Text_Hold.Read(exINI, section, "Text.Hold");
	this->Text_Charging.Read(exINI, section, "Text.Charging");
	this->Text_Active.Read(exINI, section, "Text.Active");

	// the fallback is handled in the PreDependent SW's code
	if(pINI->ReadString(section, "SW.PostDependent", Ares::readDefval, Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->SW_PostDependent, Ares::readBuffer);
	}

	// initialize the NewSWType that handles this SWType.
	if(auto pNewSWType = this->GetNewSWType()) {
		pThis->Action = this->LastAction;
		pNewSWType->LoadFromINI(this, pThis, pINI);
		this->LastAction = pThis->Action;

		// whatever the user does, we take care of the stupid tags.
		// there is no need to have them not hardcoded.
		SuperWeaponFlags::Value flags = pNewSWType->Flags();
		pThis->PreClick = ((flags & SuperWeaponFlags::PreClick) != 0);
		pThis->PostClick = ((flags & SuperWeaponFlags::PostClick) != 0);
		pThis->PreDependent = -1;
	}

	this->CameoPal.LoadFromINI(pINI, pThis->ID, "SidebarPalette");

	if(pINI->ReadString(section, "SidebarPCX", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->SidebarPCX, Ares::readBuffer);
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
		bool isWater = (pCell->LandType == LandType::Water);
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

bool SWTypeExt::ExtData::CanFireAt(HouseClass* pOwner, const CellStruct &Coords) {
	if(CellClass *pCell = MapClass::Instance->GetCellAt(Coords)) {

		// check cell type
		if(!IsCellEligible(pCell, this->SW_RequiresTarget)) {
			return false;
		}

		// check for techno type match
		TechnoClass *pTechno = generic_cast<TechnoClass*>(pCell->GetContent());
		if(pTechno && this->SW_RequiresHouse != SuperWeaponAffectedHouse::None) {
			if(!IsHouseAffected(pOwner, pTechno->Owner, this->SW_RequiresHouse)) {
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

bool SWTypeExt::Launch(SuperClass* pThis, NewSWType* pSW, const CellStruct &Coords, bool IsPlayer) {
	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type)) {

		// launch the SW, then play sounds and animations. if the SW isn't launched
		// nothing will be played.
		if(pSW->Activate(pThis, Coords, IsPlayer)) {
			SuperWeaponFlags::Value flags = pSW->Flags();

			if(flags & SuperWeaponFlags::PostClick) {
				// use the properties of the originally fired SW
				if(HouseExt::ExtData *pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
					if(auto pLast = pThis->Owner->Supers.GetItemOrDefault(pExt->SWLastIndex)) {
						pThis = pLast;
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
					Debug::Log("House %d gets %d credits\n", pThis->Owner->ArrayIndex, Money_Amount);
					pThis->Owner->GiveMoney(Money_Amount);
				} else if(Money_Amount < 0) {
					Debug::Log("House %d loses %d credits\n", pThis->Owner->ArrayIndex, -Money_Amount);
					pThis->Owner->TakeMoney(-Money_Amount);
				}
			}

			CellClass *pTarget = MapClass::Instance->GetCellAt(Coords);

			CoordStruct coords = pTarget->GetCoordsWithBridge();

			auto pAnim = pData->GetAnim();
			if(pAnim && !(flags & SuperWeaponFlags::NoAnim)) {
				coords.Z += pData->SW_AnimHeight;
				AnimClass *placeholder = GameCreate<AnimClass>(pAnim, coords);
				placeholder->Invisible = !pData->IsAnimVisible(pThis->Owner);
			}

			int sound = pData->GetSound();
			if(sound && !(flags & SuperWeaponFlags::NoSound)) {
				VocClass::PlayAt(sound, coords, nullptr);
			}

			if(pData->SW_RadarEvent && !(flags & SuperWeaponFlags::NoEvent)) {
				RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
			}

			if(!(flags & SuperWeaponFlags::NoMessage)) {
				pData->PrintMessage(pData->Message_Launch, pThis->Owner);
			}

			// this sw has been fired. clean up.
			int idxThis = pThis->Owner->Supers.FindItemIndex(pThis);
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

bool SWTypeExt::ExtData::IsOriginalType() const {
	return this->AttachedToObject->Type < FIRST_SW_TYPE;
}

// is this an original type handled by a NewSWType?
bool SWTypeExt::ExtData::IsTypeRedirected() const {
	return this->HandledByNewSWType > -1;
}

int SWTypeExt::ExtData::GetTypeIndexWithRedirect() const {
	return this->IsTypeRedirected() ? this->HandledByNewSWType : this->AttachedToObject->Type;
}

int SWTypeExt::ExtData::GetNewTypeIndex() const {
	// if new type, return new type, if original type return only if handled (else it's -1).
	return this->IsOriginalType() ? this->HandledByNewSWType : this->AttachedToObject->Type;
}

NewSWType* SWTypeExt::ExtData::GetNewSWType() const {
	int TypeIdx = this->GetNewTypeIndex();

	if(TypeIdx >= FIRST_SW_TYPE) {
		return NewSWType::GetNthItem(TypeIdx);
	}

	return nullptr;
}

void SWTypeExt::ExtData::PrintMessage(const CSFText& message, HouseClass* pFirer) {
	if(message.empty()) {
		return;
	}

	int color = ColorScheme::FindIndex("Gold");
	if(this->Message_FirerColor) {
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
	MessageListClass::Instance->PrintMessage(message, color);
}

void SWTypeExt::ClearChronoAnim(SuperClass *pThis)
{
	if(pThis->Animation) {
		pThis->Animation->RemainingIterations = 0;
		pThis->Animation = nullptr;
		PointerExpiredNotification::NotifyInvalidAnim.Remove(pThis);
	}

	if(pThis->AnimationGotInvalid) {
		PointerExpiredNotification::NotifyInvalidAnim.Remove(pThis);
		pThis->AnimationGotInvalid = false;
	}
}

void SWTypeExt::CreateChronoAnim(SuperClass *pThis, CoordStruct *pCoords, AnimTypeClass *pAnimType)
{
	ClearChronoAnim(pThis);
	
	if(pAnimType && pCoords) {
		if(auto pAnim = GameCreate<AnimClass>(pAnimType, *pCoords)) {
			SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type);
			pAnim->Invisible = !pData->IsAnimVisible(pThis->Owner);
			pThis->Animation = pAnim;
			PointerExpiredNotification::NotifyInvalidAnim.Add(pThis);
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
	if(this->Lighting_Enabled) {
		auto getValue = [](Nullable<int> &item, int ScenarioClass::* pDefMember, int def) -> int {
			int value = item.Get(pDefMember ? ScenarioClass::Instance->*pDefMember : -1);
			return (value < 0) ? def : value;
		};

		ScenarioClass* scen = ScenarioClass::Instance;
		scen->AmbientTarget = getValue(this->Lighting_Ambient, this->Lighting_DefaultAmbient, scen->AmbientOriginal);
		int cG = 1000 * getValue(this->Lighting_Green, this->Lighting_DefaultGreen, scen->Green) / 100;
		int cB = 1000 * getValue(this->Lighting_Blue, this->Lighting_DefaultBlue, scen->Blue) / 100;
		int cR = 1000 * getValue(this->Lighting_Red, this->Lighting_DefaultRed, scen->Red) / 100;
		scen->RecalcLighting(cR, cG, cB, 1);
		return true;
	}

	return false;
}

WarheadTypeClass* SWTypeExt::ExtData::GetWarhead() const {
	if(auto pType = this->GetNewSWType()) {
		return pType->GetWarhead(this);
	}

	return this->SW_Warhead.Get(nullptr);
}

AnimTypeClass* SWTypeExt::ExtData::GetAnim() const {
	if(auto pType = this->GetNewSWType()) {
		return pType->GetAnim(this);
	}

	return this->SW_Anim.Get(nullptr);
}

int SWTypeExt::ExtData::GetSound() const {
	if(auto pType = this->GetNewSWType()) {
		return pType->GetSound(this);
	}

	return this->SW_Sound.Get(-1);
}

int SWTypeExt::ExtData::GetDamage() const {
	if(auto pType = this->GetNewSWType()) {
		return pType->GetDamage(this);
	}

	return this->SW_Damage.Get(0);
}

SWRange SWTypeExt::ExtData::GetRange() const {
	if(auto pType = this->GetNewSWType()) {
		return pType->GetRange(this);
	}

	return this->SW_Range;
}

double SWTypeExt::ExtData::GetChargeToDrainRatio() const {
	return this->SW_ChargeToDrainRatio.Get(RulesClass::Instance->ChargeToDrainRatio);
}

void Container<SWTypeExt>::InvalidatePointer(void *ptr, bool bRemoved) {
	AnnounceInvalidPointer(SWTypeExt::CurrentSWType, ptr);
}

// =============================
// load / save

bool Container<SWTypeExt>::Load(SuperWeaponTypeClass *pThis, IStream *pStm) {
	SWTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->SW_Anim);

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(6CE6F6, SuperWeaponTypeClass_CTOR, 5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6CEFE0, SuperWeaponTypeClass_SDDTOR, 8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, A)
{
	GET_STACK(SWTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<SWTypeExt>::PrepareStream(pItem, pStm);

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

DEFINE_HOOK_AGAIN(6CEE50, SuperWeaponTypeClass_LoadFromINI, A)
DEFINE_HOOK(6CEE43, SuperWeaponTypeClass_LoadFromINI, A)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

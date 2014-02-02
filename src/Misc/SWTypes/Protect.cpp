#include "Protect.h"
#include "../../Ares.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

bool SW_Protect::HandlesType(int type)
{
	return (type == SuperWeaponType::IronCurtain) || (type == SuperWeaponType::ForceShield);
}

AnimTypeClass* SW_Protect::GetAnim(const SWTypeExt::ExtData* pData) const {
	if(pData->SW_Anim) {
		return pData->SW_Anim;
	} else if(pData->AttachedToObject->Type == SuperWeaponType::ForceShield) {
		return RulesClass::Instance->ForceShieldInvokeAnim;
	} else {
		return RulesClass::Instance->IronCurtainInvokeAnim;
	}
}

void SW_Protect::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	int type = pSW->Type;

	// iron curtain and force shield, as well as protect
	pData->SW_AnimHeight = 5;

	if(type == SuperWeaponType::ForceShield) {
		// force shield
		pData->Protect_IsForceShield = true;
		pData->SW_RadarEvent = false;

		pData->SW_WidthOrRange = (float)RulesClass::Instance->ForceShieldRadius;
		pData->SW_Anim = RulesClass::Instance->ForceShieldInvokeAnim;

		pData->EVA_Ready = VoxClass::FindIndex("EVA_ForceShieldReady");

		pData->SW_AITargetingType = SuperWeaponAITargetingMode::ForceShield;
		pData->SW_AffectsHouse = SuperWeaponAffectedHouse::Team;
		pData->SW_AffectsTarget = SuperWeaponTarget::Building;
		pData->SW_RequiresHouse = SuperWeaponAffectedHouse::Team;
		pData->SW_RequiresTarget = SuperWeaponTarget::Building;

		pData->SW_Cursor = MouseCursor::First[MouseCursorType::ForceShield];
		pData->SW_NoCursor = MouseCursor::First[MouseCursorType::NoForceShield];
	} else {
		// iron curtain and protect
		pData->SW_WidthOrRange = 3;
		pData->SW_Height = 3;

		pData->SW_Anim = RulesClass::Instance->IronCurtainInvokeAnim;

		pData->EVA_Ready = VoxClass::FindIndex("EVA_IronCurtainReady");
		pData->EVA_Detected = VoxClass::FindIndex("EVA_IronCurtainDetected");
		pData->EVA_Activated = VoxClass::FindIndex("EVA_IronCurtainActivated");

		pData->SW_Cursor = MouseCursor::First[MouseCursorType::IronCurtain];
	}
}

void SW_Protect::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Protect_Duration.Read(exINI, section, "Protect.Duration");
	pData->Protect_PowerOutageDuration.Read(exINI, section, "Protect.PowerOutage");
	pData->Protect_PlayFadeSoundTime.Read(exINI, section, "Protect.PlayFadeSoundTime");
}

bool SW_Protect::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	if(pThis->IsCharged) {
		CellClass *pTarget = MapClass::Instance->GetCellAt(Coords);
		CoordStruct Crd = pTarget->GetCoords();

		bool isForceShield = pData->Protect_IsForceShield;

		int duration = pData->Protect_Duration.Get(isForceShield
			? RulesClass::Instance->ForceShieldDuration : RulesClass::Instance->IronCurtainDuration);

		// play start sound
		if(pSW->StartSound > -1) {
			VocClass::PlayAt(pSW->StartSound, Crd, nullptr);
		}

		// set up the special sound when the effect wears off
		if(pThis->Type->SpecialSound > -1) {
			int playFadeSoundTime = pData->Protect_PlayFadeSoundTime.Get(isForceShield
				? RulesClass::Instance->ForceShieldPlayFadeSoundTime : 0);

			pThis->SpecialSoundDuration = duration - playFadeSoundTime;
			pThis->SpecialSoundLocation = Crd;
		}

		// shut down power
		int powerOutageDuration = pData->Protect_PowerOutageDuration.Get(isForceShield
			? RulesClass::Instance->ForceShieldBlackoutDuration : 0);

		if(powerOutageDuration > 0) {
			pThis->Owner->CreatePowerOutage(powerOutageDuration);
		}

		bool force = pData->Protect_IsForceShield;
		float width = pData->SW_WidthOrRange;
		int height = pData->SW_Height;
		
		auto IronCurtain = [&](TechnoClass* pTechno) -> bool {
			// we shouldn't do anything
			if(pTechno->IsImmobilized || pTechno->IsBeingWarpedOut()) {
				return true;
			}

			// is this thing affected at all?
			if(!pData->IsHouseAffected(pThis->Owner, pTechno->Owner)) {
				return true;
			}

			if(!pData->IsTechnoAffected(pTechno)) {
				return true;
			}

			// protect this techno
			pTechno->IronCurtain(duration, pThis->Owner, force);

			return true;
		};

		// protect everything in range
		Helpers::Alex::DistinctCollector<TechnoClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(Coords, width, height, std::ref(items));
		items.for_each(IronCurtain);
	}

	return true;
}
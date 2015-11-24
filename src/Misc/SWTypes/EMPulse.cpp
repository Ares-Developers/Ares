#include "EMPulse.h"
#include "../../Ext/Building/Body.h"
#include "../../Ext/BulletType/Body.h"
#include "../../Ext/Techno/Body.h"
#include "../../Ext/TechnoType/Body.h"

#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/INIParser.h"
#include "../../Utilities/TemplateDef.h"

#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <HouseClass.h>

void SW_EMPulse::Initialize(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW)
{
	pData->SW_RangeMaximum = -1.0;
	pData->SW_RangeMinimum = 0.0;
	pData->SW_MaxCount = 1;

	pData->EMPulse_Linked = false;
	pData->EMPulse_TargetSelf = false;

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::None;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::Attack);
	pData->SW_NoCursor = MouseCursor::GetCursor(MouseCursorType::AttackOutOfRange);
}

void SW_EMPulse::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	const char* section = pSW->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	pData->EMPulse_Linked.Read(exINI, section, "EMPulse.Linked");
	pData->EMPulse_TargetSelf.Read(exINI, section, "EMPulse.TargetSelf");
	pData->EMPulse_PulseDelay.Read(exINI, section, "EMPulse.PulseDelay");
	pData->EMPulse_PulseBall.Read(exINI, section, "EMPulse.PulseBall");
	pData->EMPulse_Cannons.Read(exINI, section, "EMPulse.Cannons");

	pSW->Action = pData->EMPulse_TargetSelf ? Action::None : Actions::SuperWeaponAllowed;
}

bool SW_EMPulse::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	auto pType = pThis->Type;
	auto pData = SWTypeExt::ExtMap.Find(pType);

	if(!pData) {
		return false;
	}

	auto pOwner = pThis->Owner;
	pOwner->EMPTarget = Coords;

	// the maximum number of buildings to fire. negative means all.
	const auto Count = (pData->SW_MaxCount >= 0)
		? static_cast<size_t>(pData->SW_MaxCount)
		: std::numeric_limits<size_t>::max();

	// if linked, only one needs to be in range (and CanFireAt checked that already).
	const bool ignoreRange = pData->EMPulse_Linked || pData->EMPulse_TargetSelf;

	auto IsEligible = [=](BuildingClass* pBld) {
		return IsLaunchSiteEligible(pData, Coords, pBld, ignoreRange);
	};

	// only call on up to Count buildings that suffice IsEligible
	Helpers::Alex::for_each_if_n(pOwner->Buildings.begin(), pOwner->Buildings.end(),
		Count, IsEligible, [=](BuildingClass* pBld)
	{
		if(!pData->EMPulse_TargetSelf) {
			// set extended properties
			auto pExt = TechnoExt::ExtMap.Find(pBld);
			pExt->SuperWeapon = pThis;
			pExt->SuperTarget = MapClass::Instance->TryGetCellAt(Coords);

			// setup the cannon and start the fire mission
			pBld->FiringSWType = pType->ArrayIndex;
			pBld->QueueMission(Mission::Missile, false);
			pBld->NextMission();
		} else {
			// create a bullet and detonate immediately
			if(auto pWeapon = pBld->GetWeapon(0)->WeaponType) {
				auto pExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

				if(auto pBullet = pExt->CreateBullet(pBld, pBld, pWeapon)) {
					pBullet->SetWeaponType(pWeapon);
					pBullet->Remove();
					pBullet->Detonate(BuildingExt::GetCenterCoords(pBld));
					pBullet->Release();
				}
			}
		}
	});

	return true;
}

bool SW_EMPulse::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	// don't further question the types in this list
	if(!pSWType->EMPulse_Cannons.empty()) {
		return pSWType->EMPulse_Cannons.Contains(pBuilding->Type) && pBuilding->IsAlive
			&& pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline();
	}

	return pBuilding->Type->EMPulseCannon && NewSWType::IsLaunchSite(pSWType, pBuilding);
}

std::pair<double, double> SW_EMPulse::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	if(pSWType->EMPulse_TargetSelf) {
		// no need for any range check
		return std::make_pair(-1.0, -1.0);
	}

	if(!pBuilding) {
		// for EMP negative ranges mean default value, so this forces
		// to do the check regardless of the actual value
		return std::make_pair(0.0, 0.0);
	}

	if(auto pWeap = pBuilding->GetWeapon(0)->WeaponType) {
		// get maximum range
		double maxRange = pSWType->SW_RangeMaximum;
		if(maxRange < 0.0) {
			maxRange = pWeap->Range / 256.0;
		}

		// get minimum range
		double minRange = pSWType->SW_RangeMinimum;
		if(minRange < 0.0) {
			minRange = pWeap->MinimumRange / 256.0;
		}

		return std::make_pair(minRange, maxRange);
	}

	return NewSWType::GetLaunchSiteRange(pSWType, pBuilding);
}

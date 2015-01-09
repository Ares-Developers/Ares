#include "Body.h"
#include "../Techno/Body.h"
#include <LaserDrawClass.h>
#include "../BuildingType/Body.h"
#include "../BulletType/Body.h"
#include <BuildingClass.h>

DEFINE_HOOK(6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pBeam, EAX);

	auto const pData = WeaponTypeExt::ExtMap.Find(pWeapon);
	int const Thickness = pData->Laser_Thickness;
	if(Thickness > 1) {
		pBeam->Thickness = Thickness;
	}

	return 0;
}

DEFINE_HOOK(6FF4DE, TechnoClass_Fire_IsLaser, 6) {
	GET(TechnoClass* const, pThis, ECX);
	GET(TechnoClass* const, pTarget, EDI);
	GET(WeaponTypeClass* const, pFiringWeaponType, EBX);

	auto const idxWeapon = R->Base<int>(0xC); // don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants

	auto const pData = WeaponTypeExt::ExtMap.Find(pFiringWeaponType);
	int const Thickness = pData->Laser_Thickness;

	if(auto const pBld = abstract_cast<BuildingClass*>(pThis)) {
		auto const pTWeapon = pBld->GetTurretWeapon()->WeaponType;

		if(auto const pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty)) {

			//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
			if(Thickness == -1) {
				pLaser->Thickness = 3;
			} else {
				pLaser->Thickness = Thickness;
			}

			auto const pBldTypeData = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if(pBldTypeData->PrismForwarding.CanAttack()) {
				//is a prism tower

				if(pBld->SupportingPrisms > 0) { //Ares sets this to the longest backward chain
					//is being supported... so increase beam intensity
					if(pBldTypeData->PrismForwarding.Intensity < 0) {
						pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
					} else if(pBldTypeData->PrismForwarding.Intensity > 0) {
						pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
					}

					// always supporting
					pLaser->IsSupported = true;
				}
			}
		}
	} else {
		if(auto const pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, CoordStruct::Empty)) {
			if(Thickness == -1) {
				pLaser->Thickness = 2;
			} else {
				pLaser->Thickness = Thickness;

				// required for larger Thickness to work right
				pLaser->IsSupported = (Thickness > 3);
			}
		}
	}

	// skip all default handling
	return 0x6FF656;
}

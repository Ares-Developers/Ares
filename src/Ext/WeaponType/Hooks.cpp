#include "Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass *, pWeapon, ECX);
	GET(LaserDrawClass *, pBeam, EAX);

	auto pData = WeaponTypeExt::ExtMap.Find(pWeapon);
	int Thickness = pData->Laser_Thickness;
	if(Thickness > 1) {
		pBeam->Thickness = Thickness;
	}

	return 0;
}


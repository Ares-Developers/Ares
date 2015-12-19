#include "../Ares.h"
#include "../Ext/TechnoType/Body.h"

#include <AnimClass.h>
#include <AircraftClass.h>
#include <BulletClass.h>
#include <LocomotionClass.h>
#include <RocketLocomotionClass.h>
#include <SpawnManagerClass.h>

DEFINE_HOOK(6622E0, RocketLocomotionClass_ILocomotion_Process_CustomMissile, 6)
{
	GET(AircraftClass*, pThis, ECX);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		if(pExt->IsCustomMissile) {
			R->EAX(&pExt->CustomMissileData);
			return 0x66230A;
		}
	}

	return 0;
}

DEFINE_HOOK(66238A, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff1, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = abstract_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false);
		}
		return 0x6623F3;
	}

	return 0;
}

DEFINE_HOOK(662512, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff2, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = abstract_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false);
		}
		return 0x66257B;
	}

	return 0;
}

DEFINE_HOOK(6627E5, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff3, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = abstract_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false);
		}
		return 0x662849;
	}

	return 0;
}

DEFINE_HOOK(662D85, RocketLocomotionClass_ILocomotion_Process_CustomMissileTrailer, 6)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = abstract_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(pLocomotor->TrailerTimer.Expired()) {
			pLocomotor->TrailerTimer.Start(pExt->CustomMissileTrailerSeparation);

			if(AnimTypeClass* pType = pExt->CustomMissileTrailerAnim) {
				GameCreate<AnimClass>(pType, pOwner->Location);
			}
		}
		return 0x662E16;
	}

	return 0;
}

DEFINE_HOOK(66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);
	GET(RocketLocomotionClass*, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, ppWarhead, 0x10);
	LEA_STACK(RocketStruct**, ppRocketData, 0x14);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile) {
			*ppRocketData = &pExt->CustomMissileData;

			bool isElite = pLocomotor->SpawnerIsElite;
			*ppWarhead = (isElite ? pExt->CustomMissileEliteWarhead : pExt->CustomMissileWarhead);
			
			return 0x6630DD;
		}
	}

	return 0;
}

DEFINE_HOOK(663218, RocketLocomotionClass_Explode_CustomMissile2, 5)
{
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CoordStruct const, coords, STACK_OFFS(0x60, 0x18));

	auto const pOwner = static_cast<AircraftClass*>(pThis->LinkedTo);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if(pExt->IsCustomMissile) {
		auto const& pWeapon = pThis->SpawnerIsElite
			? pExt->CustomMissileEliteWeapon : pExt->CustomMissileWeapon;

		if(pWeapon) {
			auto const pBullet = pWeapon->Projectile->CreateBullet(
				pOwner, pOwner, pWeapon->Damage, pWeapon->Warhead, 0,
				pWeapon->Bright);

			if(pBullet) {
				pBullet->SetWeaponType(pWeapon);
				pBullet->Remove();
				pBullet->Detonate(coords);
				pBullet->UnInit();
			}

			return 0x6632CC;
		}
	}

	return 0;
}

DEFINE_HOOK(6632F2, RocketLocomotionClass_ILocomotion_MoveTo_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, EDX);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile) {
			R->EDX(&pExt->CustomMissileData);
			return 0x66331E;
		}
	}

	return 0;
}

DEFINE_HOOK(6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile) {
			R->EAX(&pExt->CustomMissileData);
			return 0x66351B;
		}
	}

	return 0;
}

DEFINE_HOOK(6B6D60, SpawnManagerClass_CTOR_CustomMissile, 6)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	if(auto pExt = TechnoTypeExt::ExtMap.Find(pSpawnManager->SpawnType)) {
		if(pExt->IsCustomMissile) {
			return 0x6B6D86;
		}
	}

	return 0;
}

DEFINE_HOOK(6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass*, pSpawnType, EAX);
	if(auto pExt = TechnoTypeExt::ExtMap.Find(pSpawnType)) {
		if(pExt->IsCustomMissile) {
			return 0x6B791F;
		}
	}

	return 0;
}

DEFINE_HOOK(6B7A72, SpawnManagerClass_Update_CustomMissile2, 6)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(int, idxSpawn, EDI);
	GET(TechnoTypeClass*, pSpawnType, EDX);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pSpawnType)) {
		if(pExt->IsCustomMissile) {
			auto pRocket = &pExt->CustomMissileData;

			auto pTimer = &pSpawnManager->SpawnedNodes.GetItem(idxSpawn)->SpawnTimer;
			pTimer->Start(pRocket->PauseFrames + pRocket->TiltFrames);

			return 0x6B7B03;
		}
	}

	return 0;
}

DEFINE_HOOK(6B752E, SpawnManagerClass_Update_CustomMissileTakeoff, 6)
{
	GET(AircraftClass*, pOwner, EDI);

	if(auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false);
		}
		return 0x6B757A;
	}

	return 0;
}

#include "../Ares.h"
#include "../Ext/TechnoType/Body.h"

#include <AnimClass.h>
#include <AircraftClass.h>
#include <LocomotionClass.h>
#include <SpawnManagerClass.h>
#include <RocketLocomotionClass.h>

DEFINE_HOOK(6622E0, RocketLocomotionClass_ILocomotion_Process_CustomMissile, 6)
{
	GET(AircraftClass*, pThis, ECX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		if(pExt->IsCustomMissile.Get()) {
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
	AircraftClass* pOwner = specific_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			AnimClass* pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pType, &pOwner->Location, 2, 1, 0x600, -10, 0);
		}
		return 0x6623F3;
	}

	return 0;
}

DEFINE_HOOK(662512, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff2, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	AircraftClass* pOwner = specific_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			AnimClass* pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pType, &pOwner->Location, 2, 1, 0x600, -10, 0);
		}
		return 0x66257B;
	}

	return 0;
}

DEFINE_HOOK(6627E5, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff3, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	AircraftClass* pOwner = specific_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			AnimClass* pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pType, &pOwner->Location, 2, 1, 0x600, -10, 0);
		}
		return 0x662849;
	}

	return 0;
}

DEFINE_HOOK(662D85, RocketLocomotionClass_ILocomotion_Process_CustomMissileTrailer, 6)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	AircraftClass* pOwner = specific_cast<AircraftClass*>(pLocomotor->LinkedTo);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(pLocomotor->TrailerTimer.Ignorable()) {
			pLocomotor->TrailerTimer.Start(pExt->CustomMissileTrailerSeparation);

			if(AnimTypeClass* pType = pExt->CustomMissileTrailerAnim) {
				AnimClass* pAnim = NULL;
				GAME_ALLOC(AnimClass, pAnim, pType, &pOwner->Location);
			}
		}
		return 0x662E16;
	}

	return 0;
}

DEFINE_HOOK(66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftClass*, pMissile, EDX);
	GET(AircraftTypeClass*, pType, ECX);
	GET(RocketLocomotionClass*, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, pWarhead, 0x10);
	LEA_STACK(RocketStruct**, pRocketData, 0x14);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			*pRocketData = &pExt->CustomMissileData;

			bool isElite = pLocomotor->SpawnerIsElite;
			*pWarhead = (isElite ? pExt->CustomMissileEliteWarhead.Get() : pExt->CustomMissileWarhead.Get());
			
			return 0x6630DD;
		}
	}

	return 0;
}

DEFINE_HOOK(6632F2, RocketLocomotionClass_ILocomotion_MoveTo_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, EDX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			R->EDX(&pExt->CustomMissileData);
			return 0x66331E;
		}
	}

	return 0;
}

DEFINE_HOOK(6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			R->EAX(&pExt->CustomMissileData);
			return 0x66351B;
		}
	}

	return 0;
}

DEFINE_HOOK(6B6D60, SpawnManagerClass_CTOR_CustomMissile, 6)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pSpawnManager->SpawnType)) {
		if(pExt->IsCustomMissile.Get()) {
			return 0x6B6D86;
		}
	}

	return 0;
}

DEFINE_HOOK(6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass*, pSpawnType, EAX);
	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pSpawnType)) {
		if(pExt->IsCustomMissile.Get()) {
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
	GET_STACK(int, unk, 0x60);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pSpawnType)) {
		if(pExt->IsCustomMissile.Get()) {
			RocketStruct* pRocket = pExt->CustomMissileData.GetEx();

			TimerStruct* pTimer = &pSpawnManager->SpawnedNodes.GetItem(idxSpawn)->SpawnTimer;
			pTimer->Start(pRocket->PauseFrames + pRocket->TiltFrames);
			pTimer->unknown = 0;

			return 0x6B7B03;
		}
	}

	return 0;
}

DEFINE_HOOK(6B752E, SpawnManagerClass_Update_CustomMissileTakeoff, 6)
{
	GET(AircraftClass*, pOwner, EDI);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type)) {
		if(AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
			AnimClass* pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pType, &pOwner->Location, 2, 1, 0x600, -10, 0);
		}
		return 0x6B757A;
	}

	return 0;
}

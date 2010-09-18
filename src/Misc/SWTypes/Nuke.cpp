#include "Nuke.h"
#include "../../Ares.h"

#include "../../Ext/Bullet/Body.h"

#include <WarheadTypeClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <BulletClass.h>
#include <YRMath.h>

SuperWeaponTypeClass* SW_NuclearMissile::CurrentNukeType;

bool SW_NuclearMissile::HandlesType(int type)
{
	return (type == SuperWeaponType::Nuke);
}

void SW_NuclearMissile::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// invalid values so NukePayload properties can override them.
	pData->SW_Damage = -1;
	pData->SW_Warhead = NULL;
	pData->SW_ActivationSound = RulesClass::Instance->DigSound;

	// default values for the original Nuke
	pData->Nuke_Payload = WeaponTypeClass::FindOrAllocate("NukePayload");
	pData->Nuke_TakeOff = RulesClass::Instance->NukeTakeOff;
	pData->Nuke_PsiWarning = AnimTypeClass::Find("PSIWARN");
	pData->Nuke_SiloLaunch = true;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_NuclearSiloDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_NuclearMissileReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_NuclearMissileLaunched");

	pData->Lighting_Ambient = &ScenarioClass::Instance->NukeAmbient;
	pData->Lighting_Red = &ScenarioClass::Instance->NukeRed;
	pData->Lighting_Green = &ScenarioClass::Instance->NukeGreen;
	pData->Lighting_Blue = &ScenarioClass::Instance->NukeBlue;
	
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Nuke;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::Nuke];
}

void SW_NuclearMissile::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	pData->Nuke_Payload.Parse(&exINI, section, "Nuke.Payload", true);
	pData->Nuke_TakeOff.Parse(&exINI, section, "Nuke.TakeOff");
	pData->Nuke_PsiWarning.Parse(&exINI, section, "Nuke.PsiWarning");
	pData->Nuke_SiloLaunch.Read(&exINI, section, "Nuke.SiloLaunch");
}

bool SW_NuclearMissile::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	if(pThis->IsCharged) {
		SuperWeaponTypeClass *pType = pThis->Type;

		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType)) {

			CellClass* pCell = MapClass::Instance->GetCellAt(pCoords);
			CoordStruct target;
			pCell->GetCoordsWithBridge(&target);

			// the nuke has two ways to fire. first the granted way used by nukes
			// collected from crates. second, the normal way firing from a silo.
			BuildingClass* pSilo = NULL;
				
			if((!pThis->Granted || !pThis->Quantity) && pData->Nuke_SiloLaunch.Get()) {
				
				// find a building type that can fire this SWType and verify the
				// player has it. don't give up, just try the other types as well.
				for(int i=0; i<BuildingTypeClass::Array->Count; ++i) {
					BuildingTypeClass *pTBld = BuildingTypeClass::Array->GetItem(i);
					if(pTBld->NukeSilo) {
						if(pTBld->SuperWeapon == pType->ArrayIndex || pTBld->SuperWeapon2 == pType->ArrayIndex) {
							// valid silo. let's see whether the firer got it.
							if(pSilo = pThis->Owner->FindBuildingOfType(pTBld->ArrayIndex, -1)) {
								break;
							}
						}

					}
				}
			}

			// via silo
			bool fired = false;
			if(pSilo) {
				Debug::Log("Nuke launched from Missile Silo, type %s.\n", pSilo->Type->ID);
				// setup the missile and start the fire mission
				pSilo->FiringSWType = pType->ArrayIndex;
				pSilo->QueueMission(mission_Missile, false);
				pSilo->NextMission();

				pThis->Owner->NukeTarget = *pCoords;
				fired = true;
			}

			if(!fired) {
				Debug::Log("Nuke launched manually.\n");
				// if we reached this, there is no silo launch. still launch a missile.
				if(WeaponTypeClass *pWeapon = pData->Nuke_Payload) {
					if(BulletTypeClass *pProjectile = pWeapon->Projectile) {
						// get damage and warhead. they are not available during
						// initialisation, so we gotta fall back now if they are invalid.
						int damage = (pData->SW_Damage < 0 ? pWeapon->Damage : pData->SW_Damage.Get());
						WarheadTypeClass *pWarhead = (!pData->SW_Warhead ? pWeapon->Warhead : pData->SW_Warhead.Get());

						if(BulletClass* pBullet = pProjectile->CreateBullet(pCell, NULL, damage, pWarhead, pWeapon->Speed, pWeapon->Bright)) {
							pBullet->SetWeaponType(pWeapon);
							if(pData->Nuke_PsiWarning.Get()) {
								pThis->Owner->PsiWarn(pCell, pBullet, pData->Nuke_PsiWarning.Get()->ID);
							}

							if(BulletExt::ExtData *pData = BulletExt::ExtMap.Find(pBullet)) {
								pData->NukeSW = pType;
							}
								
							// aim the bullet downward and put
							// it over the target area.
							if(pBullet) {
								BulletVelocity vel;
								vel.X = 0;
								vel.Y = 0;
								vel.Z = -100;

								CoordStruct high = target;
								high.Z += 20000;

								pBullet->MoveTo(&high, &vel);
								fired = true;
							}
						}
					}
				}
			}

			if(fired) {
				VocClass::PlayAt(pData->SW_ActivationSound, &target, NULL);
				pThis->Owner->ShouldRecheckTechTree = true;
				return true;			
			}

		}
	}
	return false;
}
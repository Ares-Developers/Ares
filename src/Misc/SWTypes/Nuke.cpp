#include "Nuke.h"
#include "../../Ares.h"

#include "../../Ext/Bullet/Body.h"

#include <WarheadTypeClass.h>
#include <BuildingClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <VocClass.h>
#include <VoxClass.h>
#include <YRMath.h>

#include "../../Utilities/TemplateDef.h"

SuperWeaponTypeClass* SW_NuclearMissile::CurrentNukeType = nullptr;

bool SW_NuclearMissile::HandlesType(int type)
{
	return (type == SuperWeaponType::Nuke);
}

SuperWeaponFlags::Value SW_NuclearMissile::Flags()
{
	return SuperWeaponFlags::NoEvent;
}

WarheadTypeClass* SW_NuclearMissile::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Warhead.isset()) {
		return pData->SW_Warhead;
	}
	if(pData->Nuke_Payload) {
		return pData->Nuke_Payload->Warhead;
	}
	return nullptr;
}

int SW_NuclearMissile::GetDamage(const SWTypeExt::ExtData* pData) const
{
	auto damage = pData->SW_Damage.Get(-1);
	if(damage < 0) {
		damage = pData->Nuke_Payload ? pData->Nuke_Payload->Damage : 0;
	}
	return damage;
}

void SW_NuclearMissile::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// default values for the original Nuke
	pData->Nuke_Payload = WeaponTypeClass::FindOrAllocate("NukePayload");
	pData->Nuke_PsiWarning = AnimTypeClass::Find("PSIWARN");
	pData->Nuke_SiloLaunch = true;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_NuclearSiloDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_NuclearMissileReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_NuclearMissileLaunched");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::Nuke;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::Nuke];
}

void SW_NuclearMissile::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	pData->Nuke_Payload.Read(exINI, section, "Nuke.Payload", true);
	pData->Nuke_TakeOff.Read(exINI, section, "Nuke.TakeOff");
	pData->Nuke_PsiWarning.Read(exINI, section, "Nuke.PsiWarning");
	pData->Nuke_SiloLaunch.Read(exINI, section, "Nuke.SiloLaunch");

	Debug::Log("[Nuke] basics %s: ", section);
	Debug::Log("%s, ", pData->SW_Warhead ? pData->SW_Warhead->ID : "<empty>");
	Debug::Log("%d, ", pData->SW_Damage.Get());
	Debug::Log("%s\n", pData->AttachedToObject->WeaponType ? pData->AttachedToObject->WeaponType->ID : "<empty>");

	Debug::Log("[Nuke] parsing %s: ", section);
	Debug::Log("%s, ", pData->Nuke_Payload ? pData->Nuke_Payload->ID : "<empty>");
	Debug::Log("%s, ", pData->Nuke_TakeOff ? pData->Nuke_TakeOff->ID : "<empty>");
	Debug::Log("%s, ", pData->Nuke_PsiWarning ? pData->Nuke_PsiWarning->ID : "<empty>");
	Debug::Log("%d\n", pData->Nuke_SiloLaunch.Get());	
}

bool SW_NuclearMissile::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	if(pThis->IsCharged) {
		SuperWeaponTypeClass *pType = pThis->Type;

		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType)) {

			CellClass* pCell = MapClass::Instance->GetCellAt(Coords);
			CoordStruct target = pCell->GetCoordsWithBridge();

			// the nuke has two ways to fire. first the granted way used by nukes
			// collected from crates. second, the normal way firing from a silo.
			BuildingClass* pSilo = nullptr;
				
			if((!pThis->Granted || !pThis->OneTime) && pData->Nuke_SiloLaunch) {
				
				// find a building type that can fire this SWType and verify the
				// player has it. don't give up, just try the other types as well.
				for(int i=0; i<BuildingTypeClass::Array->Count; ++i) {
					BuildingTypeClass *pTBld = BuildingTypeClass::Array->GetItem(i);
					if(pTBld->NukeSilo) {
						if(pTBld->SuperWeapon == pType->ArrayIndex || pTBld->SuperWeapon2 == pType->ArrayIndex) {
							// valid silo. let's see whether the firer got it.
							if((pSilo = pThis->Owner->FindBuildingOfType(pTBld->ArrayIndex, -1)) != nullptr) {
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

				pThis->Owner->NukeTarget = Coords;
				fired = true;
			}

			if(!fired) {
				Debug::Log("Nuke launched manually.\n");
				// if we reached this, there is no silo launch. still launch a missile.
				if(WeaponTypeClass *pWeapon = pData->Nuke_Payload) {
					if(BulletTypeClass *pProjectile = pWeapon->Projectile) {
						// get damage and warhead. they are not available during
						// initialisation, so we gotta fall back now if they are invalid.
						int damage = GetDamage(pData);
						auto pWarhead = GetWarhead(pData);

						// create a bullet and the psi warning
						if(BulletClass* pBullet = pProjectile->CreateBullet(pCell, nullptr, damage, pWarhead, pWeapon->Speed, pWeapon->Bright)) {
							pBullet->SetWeaponType(pWeapon);
							if(AnimTypeClass* pAnimType = pData->Nuke_PsiWarning) {
								pThis->Owner->PsiWarn(pCell, pBullet, pAnimType->ID);
							}

							// remember the fired SW type
							if(BulletExt::ExtData *pBulletData = BulletExt::ExtMap.Find(pBullet)) {
								pBulletData->NukeSW = pType;
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

								pBullet->MoveTo(high, vel);
								fired = true;
							}
						}
					}
				}
			}

			if(fired) {
				// allies can see the target location before the enemy does
				if(pData->SW_RadarEvent) {
					if(pThis->Owner->IsAlliedWith(HouseClass::Player)) {
						RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
					}
				}

				VocClass::PlayAt(pData->SW_ActivationSound.Get(RulesClass::Instance->DigSound), target, nullptr);
				pThis->Owner->RecheckTechTree = true;
				return true;
			}

		}
	}
	return false;
}

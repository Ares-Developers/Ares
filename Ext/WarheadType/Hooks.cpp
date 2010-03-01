#include <InfantryClass.h>
#include <IonBlastClass.h>
#include <WeaponTypeClass.h>
#include "Body.h"
#include "../Bullet/Body.h"
#include "../../Enum/ArmorTypes.h"

// feature #384: Permanent MindControl Warheads + feature #200: EMP Warheads
// attach #407 here - set TechnoClass::Flashing.Duration
// attach #561 here, reuse #407's additional hooks for colouring
DEFINE_HOOK(46920B, BulletClass_Fire, 6) {
	GET(BulletClass *, Bullet, ESI);
	WarheadTypeClass *pThis = Bullet->WH;

	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(pThis);

	CoordStruct coords;
	if (Bullet->Target) {
		Bullet->Target->GetCoords(&coords);
	} else {
		Bullet->GetCoords(&coords);
	}
	CellStruct cellCoords = MapClass::Instance->GetCellAt(&coords)->MapCoords;

	if (pData->Ripple_Radius) {
		IonBlastClass *IB;
		GAME_ALLOC(IonBlastClass, IB, coords);
		WarheadTypeExt::IonExt[IB] = pData;
	}

	if (pData->IC_Duration != 0) {
		int countCells = CellSpread::NumCells(int(Bullet->WH->CellSpread));
		for (int i = 0; i < countCells; ++i) {
			CellStruct tmpCell = CellSpread::GetCell(i);
			tmpCell += cellCoords;
			CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
			for (ObjectClass *curObj = c->GetContent(); curObj; curObj
					= curObj->NextObject) {
				if (TechnoClass *curTechno = generic_cast<TechnoClass *>(curObj)) {
					if (curTechno->IronCurtainTimer.Ignorable()) {
						if (pData->IC_Duration > 0) {
							curTechno->IronCurtain(pData->IC_Duration,
									Bullet->Owner->Owner, 0);
						}
					} else {
						if (pData->IC_Duration > 0) {
							curTechno->IronCurtainTimer.TimeLeft
									+= pData->IC_Duration;
						} else {
							if (curTechno->IronCurtainTimer.TimeLeft <= abs(
									pData->IC_Duration)) {
								curTechno->IronCurtainTimer.TimeLeft = 1;
							} else {
								curTechno->IronCurtainTimer.TimeLeft
										+= pData->IC_Duration;
							}
						}
					}
				}
			}
		}
	}

	if (Bullet->Target) {
		if (TechnoClass *pTarget = specific_cast<TechnoClass *>(Bullet->Target)) {
			TechnoTypeClass *pType = pTarget->GetTechnoType();

			if (pData->EMP_Duration) {
				EMPulseClass *placeholder;
				GAME_ALLOC(EMPulseClass, placeholder, Bullet->Target->GetCell()->MapCoords, int(pThis->CellSpread), pData->EMP_Duration, 0);
			}

			if (pData->MindControl_Permanent) {
				if (!pType || pType->ImmuneToPsionics) {
					return 0;
				}
				if (pTarget->MindControlledBy) {
					pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
				}
				pTarget->SetOwningHouse(Bullet->Owner->Owner, 1);
				pTarget->MindControlledByAUnit = 1;
				pTarget->QueueMission(mission_Guard, 0);

				coords.Z += pType->MindControlRingOffset;

				AnimClass *MCAnim;
				GAME_ALLOC(AnimClass, MCAnim, RulesClass::Instance->PermaControlledAnimationType, &coords);
				AnimClass *oldMC = pTarget->MindControlRingAnim;
				if (oldMC) {
					oldMC->UnInit();
				}
				pTarget->MindControlRingAnim = MCAnim;
				MCAnim->SetOwnerObject(pTarget);

				return 0x469AA4;
			}
		}

		BulletExt::ExtData* TheBulletExt = BulletExt::ExtMap.Find(Bullet);
		if (TheBulletExt->DamageOccupants()) {
			// the occupants have been damaged, do not damage the building (the original target)
			Bullet->Health = 0;
			Bullet->DamageMultiplier = 0;
			Bullet->Remove();
		}
	}

	return 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A87B, TemporalClass_Update_CacheWH, 6) {
	WarheadTypeExt::Temporal_WH = R->EAX<WeaponTypeClass *> ()->Warhead;
	return 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A900, TemporalClass_Update_WarpAway, 6) {
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(
			WarheadTypeExt::Temporal_WH);

	R->EDX<AnimTypeClass *> (pData->Temporal_WarpAway);
	return 0x71A906;
}

DEFINE_HOOK(517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 6) {
	GET(InfantryClass *, I, ESI);
	bool IgnoreDefenses = R->BL() != 0;

	if (!I->IsDeployed() || IgnoreDefenses) {
		return 0;
	}
	GET(WarheadTypeClass *, WH, EBP);
	GET(int *, Damage, EDI);

	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WH);

	*Damage = int(*Damage * pData->DeployedDamage);

	return WH // yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
		? 0x517FF9
		: 0x518016
	;
}

#include "Body.h"
#include <WeaponTypeClass.h>
#include "../../Enum/ArmorTypes.h"

#include <WarheadTypeClass.h>
#include <GeneralStructures.h>
#include <HouseClass.h>
#include <ObjectClass.h>
#include <BulletClass.h>
#include <IonBlastClass.h>
#include <CellClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <EMPulseClass.h>
#include <AnimClass.h>
#include "../Bullet/Body.h"

#include <Helpers/Template.h>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
Container<WarheadTypeExt> WarheadTypeExt::ExtMap;

template<> WarheadTypeExt::TT *Container<WarheadTypeExt>::SavingObject = NULL;
template<> IStream *Container<WarheadTypeExt>::SavingStream = NULL;

hash_ionExt WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = NULL;

void WarheadTypeExt::ExtData::LoadFromINIFile(WarheadTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->ID;

	INI_EX exINI(pINI);

	if(!pINI->GetSection(section)) {
		return;
	}

	// writing custom verses parser just because
	if(pINI->ReadString(section, "Verses", "", Ares::readBuffer, Ares::readLength)) {
		int idx = 0;
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			this->Verses[idx].Parse(cur);
			++idx;
			if(idx > 10) {
				break;
			}
		}
	}

	ArmorType::LoadForWarhead(pINI, pThis);

	if(pThis->MindControl) {
		this->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", this->MindControl_Permanent);
	}

	if(pThis->EMEffect) {
		this->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", this->EMP_Duration);
	}

	this->IC_Duration = pINI->ReadInteger(section, "IronCurtain.Duration", this->IC_Duration);

	if(pThis->Temporal) {
		this->Temporal_WarpAway.Parse(&exINI, section, "Temporal.WarpAway");
	}

	this->DeployedDamage = pINI->ReadDouble(section, "Damage.Deployed", this->DeployedDamage);

	this->Ripple_Radius = pINI->ReadInteger(section, "Ripple.Radius", this->Ripple_Radius);

	this->AffectsEnemies = pINI->ReadBool(section, "AffectsEnemies", this->AffectsEnemies);

	this->InfDeathAnim.Parse(&exINI, section, "InfDeathAnim");
};

void Container<WarheadTypeExt>::InvalidatePointer(void *ptr) {
	AnnounceInvalidPointerMap(WarheadTypeExt::IonExt, ptr);
	AnnounceInvalidPointer(WarheadTypeExt::Temporal_WH, ptr);
}

// =============================
// load/save
void Container<WarheadTypeExt>::Save(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;
		pData->Verses.Save(pStm);
	}
}
/*
		pStm->Write(&IonBlastClass::Array->Count, 4, &out);
		for(int ii = 0; ii < IonBlastClass::Array->Count; ++ii) {
			IonBlastClass *ptr = IonBlastClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WarheadTypeExt::IonExt[ptr], 4, &out);
		}
*/
void Container<WarheadTypeExt>::Load(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	pData->Verses.Load(pStm, 0);

	SWIZZLE(pData->Temporal_WarpAway);
}

/*!
	This function checks if the passed warhead has Ripple.Radius set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param pWH The warhead to check/handle.
	\param coords The coordinates of the warhead impact, the center of the Ripple area.
*/
static void WarheadTypeExt::ExtData::applyRipples(WarheadTypeClass* pWH, const CoordStruct& coords) {
	WarheadTypeExt::ExtData *pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (WHExt->Ripple_Radius) {
		IonBlastClass *IB;
		GAME_ALLOC(IonBlastClass, IB, coords);
		WarheadTypeExt::IonExt[IB] = WHExt;
	}
}

/*!
	This function checks if the passed warhead has IronCurtain.Duration set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param pWH The warhead to check/handle.
	\param coords The coordinates of the warhead impact, the center of the Iron Curtain area.
	\param Owner Owner of the Iron Curtain effect, i.e. the one triggering this.
*/
static void WarheadTypeExt::ExtData::applyIronCurtain(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Owner) {
	WarheadTypeExt::ExtData *pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	CellStruct cellCoords = MapClass::Instance->GetCellAt(&coords)->MapCoords;

	if (pWHExt->IC_Duration != 0) {
		int countCells = CellSpread::NumCells(int(pWH->CellSpread));
		for (int i = 0; i < countCells; ++i) {
			CellStruct tmpCell = CellSpread::GetCell(i);
			tmpCell += cellCoords;
			CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
			for (ObjectClass *curObj = c->GetContent(); curObj; curObj
					= curObj->NextObject) {
				if (TechnoClass *curTechno = generic_cast<TechnoClass *>(curObj)) {
					if (curTechno->IronCurtainTimer.Ignorable()) {
						if (pWHExt->IC_Duration > 0) {
							curTechno->IronCurtain(pWHExt->IC_Duration, Owner, 0);
						}
					} else {
						if (pWHExt->IC_Duration > 0) {
							curTechno->IronCurtainTimer.TimeLeft
									+= pWHExt->IC_Duration;
						} else {
							if (curTechno->IronCurtainTimer.TimeLeft <= abs(
									pWHExt->IC_Duration)) {
								curTechno->IronCurtainTimer.TimeLeft = 1;
							} else {
								curTechno->IronCurtainTimer.TimeLeft
										+= pWHExt->IC_Duration;
							}
						}
					}
				}
			}
		}
	}
}

/*!
	This function checks if the passed warhead has EMP.Duration set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param pWH The warhead to check/handle.
	\param coords The coordinates of the warhead impact, the center of the EMP area.
*/
static void WarheadTypeExt::ExtData::applyEMP(WarheadTypeClass* pWH, const CoordStruct& coords) {
	WarheadTypeExt::ExtData *pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	if (pWHExt->EMP_Duration) {
		CellStruct cellCoords = MapClass::Instance->GetCellAt(&coords)->MapCoords;

		EMPulseClass *placeholder;
		GAME_ALLOC(EMPulseClass, placeholder, cellCoords, int(pWH->CellSpread), pWHExt->EMP_Duration, 0);
	}
}

/*!
	This function checks if the passed warhead has MindControl.Permanent set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param pWH The warhead to check/handle.
	\param coords The coordinates of the warhead impact, the center of the Mind Control animation.
	\param Owner Owner of the Mind Control effect, i.e. the one controlling the target afterwards.
	\param Target Target of the Mind Control effect, i.e. the one being controlled by the owner afterwards.
	\return false if effect wasn't applied, true if it was. This is important for the chain of damage effects, as, in case of true, the target is now a friendly unit.
*/
static bool WarheadTypeExt::ExtData::applyPermaMC(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Owner, ObjectClass* Target) {
	WarheadTypeExt::ExtData *pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt->MindControl_Permanent && Target) {
		if (TechnoClass *pTarget = generic_cast<TechnoClass *>(Target)) {
			TechnoTypeClass *pType = pTarget->GetTechnoType();

			if (!pType || pType->ImmuneToPsionics) {
				return false; // should return 0 in hook
			}
			if (pTarget->MindControlledBy) {
				pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
			}
			pTarget->SetOwningHouse(Owner, 1);
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
			return true; // should return 0x469AA4 in hook
		}
	}
}

/*!
	This function checks if the projectile transporting the warhead should pass through the building's walls and deliver the warhead to the occupants instead. If so, it performs that effect.
	\note Moved here from hook BulletClass_Fire.
	\note This cannot logically be triggered in situations where the warhead is not delivered by a projectile, such as the GenericWarhead super weapon.
	\param Bullet The projectile
	\todo This should probably be moved to /Ext/Bullet/ instead, I just maintained the previous structure to ease transition. Since UC.DaMO (#778) in 0.5 will require a reimplementation of this logic anyway, we can probably just leave it here until then.
*/
static void WarheadTypeExt::ExtData::applyOccupantDamage(BulletClass* Bullet) {
	if (Bullet->Target) {
		BulletExt::ExtData* TheBulletExt = BulletExt::ExtMap.Find(Bullet);
		if (TheBulletExt->DamageOccupants()) {
			// the occupants have been damaged, do not damage the building (the original target)
			Bullet->Health = 0;
			Bullet->DamageMultiplier = 0;
			Bullet->Remove();
		}
	}
}

// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E510, WarheadTypeClass_DTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ECX);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(75E0C0, WarheadTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK_AGAIN(75E2C0, WarheadTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(WarheadTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<WarheadTypeExt>::SavingObject = pItem;
	Container<WarheadTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(75E2AE, WarheadTypeClass_Load_Suffix, 7)
{
	WarheadTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save_Suffix, 5)
{
	WarheadTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

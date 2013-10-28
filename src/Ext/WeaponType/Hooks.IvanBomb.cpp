#include <BombListClass.h>

#include "Body.h"
#include "../Techno/Body.h"

#include <Helpers/Other.h>

// custom ivan bomb attachment
// bugfix #385: Only InfantryTypes can use Ivan Bombs
DEFINE_HOOK(438E86, BombListClass_Plant_AllTechnos, 5)
{
	GET(TechnoClass *, Source, EBP);
	switch(Source->WhatAmI()) {
		case abs_Aircraft:
		case abs_Infantry:
		case abs_Unit:
		case abs_Building:
			return 0x438E97;
		default:
			return 0x439022;
	}
}

DEFINE_HOOK(438FD7, BombListClass_Plant_AttachSound, 7)
{
	return 0x439022;
}

// 6F5230, 5
// custom ivan bomb drawing 1
DEFINE_HOOK(6F5230, TechnoClass_DrawExtras1, 5)
{
	GET(TechnoClass *, pThis, EBP);
	BombClass * Bomb = pThis->AttachedBomb;

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];

	if(pData->Ivan_Image.Get()->Frames < 2) {
		R->EAX(0);
		return 0x6F5235;
	}

	int frame =
	(Unsorted::CurrentFrame - Bomb->PlantingFrame)
		/ (pData->Ivan_Delay.Get() / (pData->Ivan_Image.Get()->Frames - 1)); // -1 so that last iteration has room to flicker

	if(Unsorted::CurrentFrame % (2 * pData->Ivan_FlickerRate) >= pData->Ivan_FlickerRate) {
		++frame;
	}

	if(frame >= pData->Ivan_Image.Get()->Frames) {
		frame = pData->Ivan_Image.Get()->Frames - 1;
	} else if(frame == pData->Ivan_Image.Get()->Frames - 1) {
		--frame;
	}

	R->EAX(frame);

	return 0x6F5235;
}

// 6F523C, 5
// custom ivan bomb drawing 2
DEFINE_HOOK(6F523C, TechnoClass_DrawExtras2, 5)
{
	GET(TechnoClass *, pThis, EBP);
	BombClass * Bomb = pThis->AttachedBomb;

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];

	if(SHPStruct *Image = pData->Ivan_Image.Get()) {
		R->ECX(Image);
		return 0x6F5247;
	}
	return 0;
}

// 6FCBAD, 6
// custom ivan bomb disarm 1
DEFINE_HOOK(6FCBAD, TechnoClass_GetObjectActivityState_IvanBomb, 6)
{
	GET(TechnoClass *, Target, EBP);
	GET(WarheadTypeClass *, Warhead, EDI);
	if(Warhead->BombDisarm) {
		if(BombClass *Bomb = Target->AttachedBomb) {
			WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];
			if(!pData->Ivan_Detachable) {
				return 0x6FCBBE;
			}
		}
	}
	return 0;
}

// 51E488, 5
DEFINE_HOOK(51E488, InfantryClass_GetCursorOverObject2, 5)
{
	GET(TechnoClass *, Target, ESI);
	BombClass *Bomb = Target->AttachedBomb;

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];
	if(!pData->Ivan_Detachable) {
		return 0x51E49E;
	}
	return 0;
}

// 438799, 6
// custom ivan bomb detonation 1
DEFINE_HOOK(438799, BombClass_Detonate1, 6)
{
	GET(BombClass *, Bomb, ESI);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];

	R->Stack<WarheadTypeClass *>(0x4, pData->Ivan_WH);
	R->EDX(pData->Ivan_Damage);
	return 0x43879F;
}

// 438843, 6
// custom ivan bomb detonation 2
DEFINE_HOOK(438843, BombClass_Detonate2, 6)
{
	GET(BombClass *, Bomb, ESI);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];

	R->EDX<WarheadTypeClass *>(pData->Ivan_WH);
	R->ECX(pData->Ivan_Damage);
	return 0x438849;
}

// 438879, 6
// custom ivan bomb detonation 3
DEFINE_HOOK(438879, BombClass_Detonate3, 6)
{
	GET(BombClass *, Bomb, ESI);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BombExt[Bomb];
	return pData->Ivan_KillsBridges ? 0 : 0x438989;
}

// 4393F2, 5
// custom ivan bomb cleanup
DEFINE_HOOK(4393F2, BombClass_SDDTOR, 5)
{
	GET(BombClass *, Bomb, ECX);
	hash_bombExt::iterator i = WeaponTypeExt::BombExt.find(Bomb);
	if(i != WeaponTypeExt::BombExt.end()) {
		WeaponTypeExt::BombExt[Bomb] = 0;
		WeaponTypeExt::BombExt.erase(Bomb);
	}
	return 0;
}

/* this is a wtf: it unsets target if the unit can no longer affect its current target.
 * Makes sense, except Aircraft that lose the target so crudely in the middle of the attack
 * (i.e. ivan bomb weapon) go wtfkerboom with an IE
 */
DEFINE_HOOK(6FA4C6, TechnoClass_Update_ZeroOutTarget, 5)
{
	GET(TechnoClass *, T, ESI);
	return (T->WhatAmI() == abs_Aircraft) ? 0x6FA4D1 : 0;
}

DEFINE_HOOK(46934D, IvanBombs_Spread, 6)
{
	GET(BulletClass *, pBullet, ESI);
	double cSpread = pBullet->WH->CellSpread;

	RET_UNLESS(pBullet->Owner && pBullet->Target);

	TechnoClass *pOwner = generic_cast<TechnoClass *>(pBullet->Owner);
	if(!pOwner) {
		return 0x469AA4;
	}

	TechnoClass *pTarget = generic_cast<TechnoClass *>(pBullet->Target);

	auto pExt = WeaponTypeExt::ExtMap.Find(pBullet->GetWeaponType());

	// just real target
	if(cSpread < 0.5) {
		if(pTarget) {
			pExt->PlantBomb(pOwner, pTarget);
		}
		return 0x469AA4;
	}

	int Spread = int(cSpread);

	CoordStruct tgtCoords;
	pBullet->GetTargetCoords(&tgtCoords);

	CellStruct centerCoords = MapClass::Instance->GetCellAt(&tgtCoords)->MapCoords;

	class IvanBombSpreadApplicator : public CellSpreadApplicator {
		protected:
			TechnoClass *pOwner;
			WeaponTypeExt::ExtData* pWeaponExt;
		public:
			IvanBombSpreadApplicator(TechnoClass *owner, WeaponTypeExt::ExtData* pWeaponExt)
				: pOwner(owner), pWeaponExt(pWeaponExt), CellSpreadApplicator()
			{ }
			virtual void operator() (ObjectClass *curObj, CellStruct *origin) override {
				if(curObj != pOwner && !curObj->AttachedBomb) {
					if(TechnoClass *curTech = generic_cast<TechnoClass *>(curObj)) {
						pWeaponExt->PlantBomb(pOwner, curTech);
					}
				}
			}
	} BombSpreader(pOwner, pExt);

	CellSpreadIterator BombDelivery(BombSpreader, &centerCoords, Spread);
	BombDelivery.Apply();

/*
	int countCells = CellSpread::NumCells(Spread);

	for(int i = 0; i < countCells; ++i) {
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += centerCoords;
		CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);

		for(ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
			if(curObj != pOwner && (curObj->AbstractFlags & ABSFLAGS_ISTECHNO) && !curObj->AttachedBomb) {
				BombListClass::Global()->Plant(pOwner, reinterpret_cast<TechnoClass *>(curObj));
			}
		}
	}
*/

	return 0x469AA4;
}

// deglobalized manual detonation settings
DEFINE_HOOK(6FFFB1, TechnoClass_GetCursorOverObject_IvanBombs, 8)
{
	GET(TechnoClass*, pThis, EDI);
	auto pBomb = pThis->AttachedBomb;
	auto pExt = WeaponTypeExt::BombExt[pBomb];

	bool canDetonate = (pBomb->IsDeathBomb() == FALSE)
		? pExt->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb)
		: pExt->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);
	return canDetonate ? 0x6FFFCC : 0x700006;
}

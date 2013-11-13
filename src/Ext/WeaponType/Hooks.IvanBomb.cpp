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

DEFINE_HOOK(438A00, BombClass_GetCurrentFrame, 6)
{
	GET(BombClass*, pThis, ECX);

	auto pData = WeaponTypeExt::BombExt[pThis];
	if(!pData) {
		return 0;
	}

	SHPStruct* pSHP = pData->Ivan_Image;
	int frame = 0;

	if(pSHP->Frames >= 2) {
		if(pThis->DeathBomb == FALSE) {
			// -1 so that last iteration has room to flicker. order is important
			int lifetime = (Unsorted::CurrentFrame - pThis->PlantingFrame);
			frame = lifetime / (pData->Ivan_Delay / (pSHP->Frames - 1));

			// flicker over a time period
			int period = 2 * pData->Ivan_FlickerRate;
			if(Unsorted::CurrentFrame % period >= pData->Ivan_FlickerRate) {
				++frame;
			}

			if(frame >= pSHP->Frames) {
				frame = pSHP->Frames - 1;
			} else if(frame == pSHP->Frames - 1) {
				--frame;
			}
		} else {
			// DeathBombs (that don't exist) use the last frame
			frame = pSHP->Frames - 1;
		}
	}

	R->EAX(frame);
	return 0x438A62;
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

	CellSpreadIterator iter(centerCoords, Spread);
	iter.apply<TechnoClass>([pOwner, pExt](TechnoClass* pTechno) {
		if(pTechno != pOwner && !pTechno->AttachedBomb) {
			pExt->PlantBomb(pOwner, pTechno);
		}
		return true;
	});

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

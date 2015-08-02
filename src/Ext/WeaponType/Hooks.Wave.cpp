#include "Body.h"
#include "../Techno/Body.h"

#include <HouseClass.h>
#include <WaveClass.h>

// custom beam styles
// 6FF5F5, 6
DEFINE_HOOK(6FF5F5, TechnoClass_Fire, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pSource, EBX);
	GET(TechnoClass* const, pTarget, EDI);

	auto const pData = WeaponTypeExt::ExtMap.Find(pSource);

	if(!pData->IsWave()) {
		return 0;
	}

	GET_BASE(byte, idxWeapon, 0xC);

	TechnoExt::ExtMap.Find(pThis)->idxSlot_Wave = idxWeapon;

	if(!pData->Wave_IsLaser && !pData->Wave_IsBigLaser) {
		return 0;
	}

	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct const, crdTgt, 0x88);

	auto const type = pData->Wave_IsBigLaser
		? WaveType::BigLaser : WaveType::Laser;

	auto const pWave = GameCreate<WaveClass>(
		crdSrc, crdTgt, pThis, type, pTarget);

	WeaponTypeExt::WaveExt[pWave] = pData;
	pThis->Wave = pWave;
	return 0x6FF650;
}

// 75E963, 6
DEFINE_HOOK(75E963, WaveClass_CTOR, 6)
{
	GET(WaveClass *, Wave, ESI);
	GET(WaveType, Type, ECX);
	if(Type == WaveType::Laser || Type == WaveType::BigLaser) {
		return 0;
	}
	GET(WeaponTypeClass *, Weapon, EBX);

	if(Weapon) {
		WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(Weapon);
		WeaponTypeExt::WaveExt[Wave] = pData;
	}
	return 0;
}

/*
// 75EB87, 0A // fsdblargh, a single instruction spanning 10 bytes
XPORT_FUNC(WaveClass_CTOR2)
{
	GET(WaveClass *, Wave, ESI);
	RET_UNLESS(CONTAINS(WeaponTypeExt::WaveExt, Wave));
	WeaponTypeExt::WeaponTypeClassData *pData = WeaponTypeExt::WaveExt[Wave];
//	Wave->set_WaveIntensity(pData->Wave_InitialIntensity);
	return 0x75EB91;
}
*/

// 763226, 6
DEFINE_HOOK(763226, WaveClass_DTOR, 6)
{
	GET(WaveClass *, Wave, EDI);
	WeaponTypeExt::WaveExt.erase(Wave);
	return 0;
}

// 760F50, 6
// complete replacement for WaveClass::Update
DEFINE_HOOK(760F50, WaveClass_Update, 6)
{
	GET(WaveClass *, pThis, ECX);

	auto pData = WeaponTypeExt::WaveExt.get_or_default(pThis);
	const WeaponTypeClass *Weap = pData->OwnerObject();

	if(!Weap) {
		return 0;
	}

	int Intensity;

	if(Weap->AmbientDamage) {
		CoordStruct coords;
//		Debug::Log("Damaging Cells for weapon %X (Intensity = %d)\n", pData, pThis->WaveIntensity);
		for(int i = 0; i < pThis->Cells.Count; ++i) {
			CellClass *Cell = pThis->Cells.GetItem(i);
//			Debug::Log("\t(%hd,%hd)\n", Cell->MapCoords.X, Cell->MapCoords.Y);
			pThis->DamageArea(*Cell->Get3DCoords3(&coords));
		}
//		Debug::Log("Done damaging %X\n", pData);
	}

	switch(pThis->Type) {
		case WaveType::Sonic:
			pThis->Update_Wave();
			Intensity = pThis->WaveIntensity;
			--Intensity;
			pThis->WaveIntensity = Intensity;
			if(Intensity < 0) {
				pThis->UnInit();
			} else {
				SET_REG32(ECX, pThis);
				CALL(0x5F3E70); // ObjectClass::Update
			}
			break;
		case WaveType::BigLaser:
		case WaveType::Laser:
			Intensity = pThis->LaserIntensity;
			Intensity -= 6;
			pThis->LaserIntensity = Intensity;
			if(Intensity < 32) {
				pThis->UnInit();
			}
			break;
		case WaveType::Magnetron:
			pThis->Update_Wave();
			Intensity = pThis->WaveIntensity;
			--Intensity;
			pThis->WaveIntensity = Intensity;
			if(Intensity < 0) {
				pThis->UnInit();
			} else {
				SET_REG32(ECX, pThis);
				CALL(0x5F3E70); // ObjectClass::Update
			}
			break;
	}

	return 0x76101A;
}

/*
// 760FFC, 5
// Alt beams update
XPORT_FUNC(WaveClass_UpdateLaser)
{
	GET(WaveClass *, Wave, ESI);
	Wave->Update_Beam();
	RET_UNLESS(CONTAINS(WeaponTypeExt::WaveExt, Wave));
	WeaponTypeExt::WeaponTypeClassData *pData = WeaponTypeExt::WaveExt[Wave];
	int intense = Wave->get_WaveIntensity() + pData->Wave_IntensityStep;
	Wave->set_WaveIntensity(intense);
	return intense >= pData->Wave_FinalIntensity ? 0x761016 : 0x76100C;
}
*/

DEFINE_HOOK(760BC2, WaveClass_Draw2, 9)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EBP);

	return (WeaponTypeExt::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave))
		? 0x760CAFu
		: 0u
	;
}

// 760DE2, 6
DEFINE_HOOK(760DE2, WaveClass_Draw3, 9)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EDI);

	return (WeaponTypeExt::ModifyWaveColor(*dest, *dest, Wave->LaserIntensity, Wave))
		? 0x760ECBu
		: 0u
	;
}

// 75EE57, 7
DEFINE_HOOK(75EE57, WaveClass_Draw_Sonic, 7)
{
	GET_STACK(WaveClass *, Wave, 0x4);
	GET(WORD*, src, EDI);
	GET(DWORD, offset, ECX);

	return (WeaponTypeExt::ModifyWaveColor(src[offset], *src, R->ESI(), Wave))
		? 0x75EF1Cu
		: 0u
	;
}

// 7601FB, 0B
DEFINE_HOOK(7601FB, WaveClass_Draw_Magnetron, 0B)
{
	GET_STACK(WaveClass *, Wave, 0x8);
	GET(WORD*, src, EBX);
	GET(DWORD, offset, ECX);

	return (WeaponTypeExt::ModifyWaveColor(src[offset], *src, R->EBP(), Wave))
		? 0x760285u
		: 0u
	;
}

// 760286, 5
DEFINE_HOOK(760286, WaveClass_Draw_Magnetron2, 5)
{
	return 0x7602D3;
}

// 762C5C, 6
DEFINE_HOOK(762C5C, WaveClass_Update_Wave, 6)
{
	GET(WaveClass *, Wave, ESI);
	TechnoClass *Firer = Wave->Owner;
	TechnoClass *Target = Wave->Target;
	if(!Target || !Firer) {
		return 0x762D57;
	}

	auto pData = WeaponTypeExt::WaveExt.get_or_default(Wave);

	if(!pData) {
		return 0;
	}

	int weaponIdx = TechnoExt::ExtMap.Find(Firer)->idxSlot_Wave;

	CoordStruct xyzSrc = Firer->GetFLH(weaponIdx, CoordStruct::Empty);
	CoordStruct xyzTgt = Target->GetCoords__(); // not GetCoords() !

	bool reversed = pData->IsWaveReversedAgainst(Target);

	if(Wave->Type == WaveType::Magnetron) {
		reversed
			? Wave->Draw_Magnetic(xyzTgt, xyzSrc)
			: Wave->Draw_Magnetic(xyzSrc, xyzTgt);
	} else {
		reversed
			? Wave->Draw_NonMagnetic(xyzTgt, xyzSrc)
			: Wave->Draw_NonMagnetic(xyzSrc, xyzTgt);
	}

	return 0x762D57;
}

// 75F38F, 6
DEFINE_HOOK(75F38F, WaveClass_DamageCell, 6)
{
	GET(WaveClass *, Wave, EBP);
	auto pData = WeaponTypeExt::WaveExt.get_or_default(Wave);
	R->EDI(R->EAX());
	R->EBX(pData->OwnerObject());
	return 0x75F39D;
}

DEFINE_HOOK(7601C7, WaveClass_Draw_Purple, 8)
{
	GET(int, Q, EDX);
	if(Q > 0x15F90) {
		Q = 0x15F90;
	}
	R->EDX(Q);
	return 0;
}

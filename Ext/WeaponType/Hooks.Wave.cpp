#include "Body.h"
#include "../Techno/Body.h"

// custom beam styles
// 6FF5F5, 6
DEFINE_HOOK(6FF5F5, TechnoClass_Fire, 6)
{
	GET(WeaponTypeClass *, Source, EBX);
	GET(TechnoClass *, Owner, ESI);
	GET(TechnoClass *, Target, EDI);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(Source);

	RET_UNLESS(Source->IsMagBeam || Source->IsSonic || pData->Wave_IsLaser || pData->Wave_IsBigLaser);

	GET_BASE(byte, idxWeapon, 0xC);

	TechnoExt::ExtMap.Find(Owner)->idxSlot_Wave = idxWeapon;

	RET_UNLESS(pData->Wave_IsLaser || pData->Wave_IsBigLaser);

	LEA_STACK(CoordStruct *, xyzSrc, 0x44);
	LEA_STACK(CoordStruct *, xyzTgt, 0x88);

	WaveClass *Wave;
	GAME_ALLOC(WaveClass, Wave, xyzSrc, xyzTgt, Owner, pData->Wave_IsBigLaser ? 2 : 1, Target);

	WeaponTypeExt::WaveExt[Wave] = pData;
	Owner->Wave = Wave;
	return 0x6FF650;
}

// 75E963, 6
DEFINE_HOOK(75E963, WaveClass_CTOR, 6)
{
	GET(WaveClass *, Wave, ESI);
	GET(DWORD, Type, ECX);
	if(Type == wave_Laser || Type == wave_BigLaser) {
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
	hash_waveExt::iterator i = WeaponTypeExt::WaveExt.find(Wave);
	if(i != WeaponTypeExt::WaveExt.end()) {
		WeaponTypeExt::WaveExt.erase(i);
	}

	return 0;
}

// 760F50, 6
// complete replacement for WaveClass::Update
DEFINE_HOOK(760F50, WaveClass_Update, 6)
{
	GET(WaveClass *, pThis, ECX);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::WaveExt[pThis];
	const WeaponTypeClass *Weap = pData->AttachedToObject;

	RET_UNLESS(Weap);

	int Intensity;

	if(Weap->AmbientDamage) {
		CoordStruct coords;
//		Debug::Log("Damaging Cells for weapon %X (Intensity = %d)\n", pData, pThis->WaveIntensity);
		for(int i = 0; i < pThis->get_Cells()->Count; ++i) {
			CellClass *Cell = pThis->get_Cells()->GetItem(i);
//			Debug::Log("\t(%hd,%hd)\n", Cell->MapCoords.X, Cell->MapCoords.Y);
			pThis->DamageArea(Cell->Get3DCoords3(&coords));
		}
//		Debug::Log("Done damaging %X\n", pData);
	}

	switch(pThis->Type) {
		case wave_Sonic:
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
		case wave_BigLaser:
		case wave_Laser:
			Intensity = pThis->LaserIntensity;
			Intensity -= 6;
			pThis->LaserIntensity = Intensity;
			if(Intensity < 32) {
				pThis->UnInit();
			}
			break;
		case wave_Magnetron:
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

DEFINE_HOOK(760BC2, WaveClass_Draw2, 0)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EBP);

	WeaponTypeExt::ModifyWaveColor(dest, dest, Wave->LaserIntensity, Wave);

	return 0x760CAF;
}

// 760DE2, 6
DEFINE_HOOK(760DE2, WaveClass_Draw3, 0)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EDI);

	WeaponTypeExt::ModifyWaveColor(dest, dest, Wave->LaserIntensity, Wave);

	return 0x760ECB;
}

// 75EE57, 7
DEFINE_HOOK(75EE57, WaveClass_Draw_Sonic, 7)
{
	GET_STACK(WaveClass *, Wave, 0x4);
	GET(DWORD, src, EDI);
	GET(DWORD, offset, ECX);
	DWORD offs = src + offset * 2;

	WeaponTypeExt::ModifyWaveColor((WORD *)offs, (WORD *)src, R->ESI(), Wave);

	return 0x75EF1C;
}

// 7601FB, 0B
DEFINE_HOOK(7601FB, WaveClass_Draw_Magnetron, 0B)
{
	GET_STACK(WaveClass *, Wave, 0x8);
	GET(DWORD, src, EBX);
	GET(DWORD, offset, ECX);
	DWORD offs = src + offset * 2;

	WeaponTypeExt::ModifyWaveColor((WORD *)offs, (WORD *)src, R->EBP(), Wave);

	return 0x760285;
}

// 760286, 5
DEFINE_HOOK(760286, WaveClass_Draw_Magnetron2, 5)
{
	return 0x7602D3;
}

void WeaponTypeExt::ModifyWaveColor(WORD *src, WORD *dst, int Intensity, WaveClass *Wave)
{
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::WaveExt[Wave];

	ColorStruct *CurrentColor = (pData->Wave_IsHouseColor && Wave->Owner)
		? &Wave->Owner->Owner->Color
		: &pData->Wave_Color;

	ColorStruct initial = Drawing::WordColor(*src);

	ColorStruct modified = initial;

// ugly hack to fix byte wraparound problems
#define upcolor(c) \
	int _ ## c = initial. c + (Intensity * CurrentColor-> c ) / 256; \
	_ ## c = min(_ ## c, 255); \
	modified. c = (BYTE)_ ## c;

	upcolor(R);
	upcolor(G);
	upcolor(B);

	WORD color = Drawing::Color16bit(&modified);

	*dst = color;
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

	RET_UNLESS(WeaponTypeExt::WaveExt.find(Wave) != WeaponTypeExt::WaveExt.end());
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::WaveExt[Wave];
	int weaponIdx = TechnoExt::ExtMap.Find(Firer)->idxSlot_Wave;

	CoordStruct xyzSrc, xyzTgt, xyzDummy = {0, 0, 0};
	Firer->GetFLH(&xyzSrc, weaponIdx, xyzDummy);
	Target->GetCoords__(&xyzTgt); // not GetCoords() ! 

	char idx = WeaponTypeExt:: AbsIDtoIdx(Target->WhatAmI());

	bool reversed = pData->Wave_Reverse[idx];

	if(Wave->Type == wave_Magnetron) {
		reversed
			? Wave->Draw_Magnetic(&xyzTgt, &xyzSrc)
			: Wave->Draw_Magnetic(&xyzSrc, &xyzTgt);
	} else {
		reversed
			? Wave->Draw_NonMagnetic(&xyzTgt, &xyzSrc)
			: Wave->Draw_NonMagnetic(&xyzSrc, &xyzTgt);
	}

	return 0x762D57;
}

// 75F38F, 6
DEFINE_HOOK(75F38F, WaveClass_DamageCell, 6)
{
	GET(WaveClass *, Wave, EBP);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::WaveExt[Wave];
	R->EDI(R->EAX());
	R->EBX(pData->AttachedToObject);
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

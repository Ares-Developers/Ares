#include <YRPP.h>
#include "WeaponTypeExt.h"

EXT_P_DECLARE(WeaponTypeClass);

void __stdcall WeaponTypeClassExt::Create(WeaponTypeClass* pThis)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);
		pData->IsCustom     = 0;
		pData->BeamColor    = ColorStruct(255, 255, 255);
		pData->BeamPeriod = 15;
		pData->BeamAmplitude = 40.0;
		Ext_p[pThis] = pData;
	}
}

void __stdcall WeaponTypeClassExt::Delete(WeaponTypeClass* pThis)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

void __stdcall WeaponTypeClassExt::Load(WeaponTypeClass* pThis, IStream* pStm)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void __stdcall WeaponTypeClassExt::Save(WeaponTypeClass* pThis, IStream* pStm)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void __stdcall WeaponTypeClassExt::LoadFromINI(WeaponTypeClass* pThis, CCINIClass* pINI)
{
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(pThis->get_ID()))
	{
		return;
	}
	
	ExtData *pData = Ext_p[pThis];

	pData->IsCustom     |= 
		pINI->IsKeySet(pThis->get_ID(), "BeamPeriod") ||
		pINI->IsKeySet(pThis->get_ID(), "BeamAmplitude") ||
		pINI->IsKeySet(pThis->get_ID(), "BeamColor");
	pData->IsCustom     &= pINI->ReadBool(pThis->get_ID(), "CustomBeam", pData->IsCustom);

	RETZ_UNLESS(pData->IsCustom);
	ColorStruct tmpColor = pData->BeamColor;
	pINI->ReadColor(&tmpColor, pThis->get_ID(), "BeamColor", &pData->BeamColor);
	pData->BeamColor     = tmpColor;

	pData->BeamPeriod = pINI->ReadInteger(pThis->get_ID(), "BeamPeriod", pData->BeamPeriod);
	pData->BeamAmplitude = pINI->ReadDouble(pThis->get_ID(), "BeamAmplitude", pData->BeamAmplitude);
}

// 6FD79C, 6
// custom RadBeam colors
EXPORT_FUNC(TechnoClass_FireRadBeam)
{
	GET(RadBeam *, Rad, ESI);
	WeaponTypeClass* Source = (WeaponTypeClass *)R->get_StackVar32(0xC);
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->IsCustom);
	Rad->set_Color(pData->BeamColor);
	Rad->set_Period(pData->BeamPeriod);
	Rad->set_Amplitude(pData->BeamAmplitude);
	return 0x6FD7A8;
}

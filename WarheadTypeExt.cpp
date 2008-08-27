#include <YRPP.h>
#include "WarheadTypeExt.h"

EXT_P_DECLARE(WarheadTypeClass);

void __stdcall WarheadTypeClassExt::Create(WarheadTypeClass* pThis)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->IsCustom     = 0;

		Ext_p[pThis] = pData;
	}
}

void __stdcall WarheadTypeClassExt::Delete(WarheadTypeClass* pThis)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

void __stdcall WarheadTypeClassExt::Load(WarheadTypeClass* pThis, IStream* pStm)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void __stdcall WarheadTypeClassExt::Save(WarheadTypeClass* pThis, IStream* pStm)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void __stdcall WarheadTypeClassExt::LoadFromINI(WarheadTypeClass* pThis, CCINIClass* pINI)
{
	const char * section = pThis->get_ID();
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	if(!pINI->ReadBool(section, "Warhead.IsCustom", 1))
	{
		pData->IsCustom = 0;
	}
	else
	{
		pData->PermaMindControl = pINI->ReadBool(pThis->get_ID(), "PermanentMindControl", pData->PermaMindControl);
/*
		char buffer[256];
		if(pINI->ReadString(section, "IvanBomb.Warhead", pData->Ivan_WH->get_ID(), buffer, 256) > 0)
		{
			pData->Ivan_WH           = WarheadTypeClass::FindOrAllocate(buffer);
		}
*/
	}
}

// 46920B, 6
EXPORT_FUNC(BulletClass_Fire)
{
	GET(BulletClass *, Bullet, ESI);
	WarheadTypeClass *pThis = Bullet->get_WH();

	RET_UNLESS(CONTAINS(WarheadTypeClassExt::Ext_p, pThis));
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[pThis];

	RET_UNLESS(pData->PermaMindControl);

	TechnoClass *pTarget = (TechnoClass *)Bullet->get_Target();
	switch(pTarget->WhatAmI())
	{
		case abs_Aircraft:
		case abs_Infantry:
		case abs_Unit:
		case abs_Building:
			break;
		default:
			return 0;
	}

	TechnoTypeClass *pType = (TechnoTypeClass *)pTarget->GetType();

	if(pType->get_ImmuneToPsionics() || pTarget->IsIronCurtained()) return 0;
	
	if(pTarget->get_MindControlledBy())
	{
		pTarget->get_CaptureManager()->FreeUnit(pTarget);
	}
	pTarget->SetOwningHouse(Bullet->get_Owner()->get_Owner(), 1);
	pTarget->set_MindControlledByAUnit(1);
	
	CoordStruct coords;
	*pTarget->GetCoords(&coords);
	coords.Z += pType->get_MindControlRingOffset();
	
	AnimClass *MCAnim = new AnimClass(RulesClass::Global()->get_PermaControlledAnimationType(), coords);
	pTarget->set_MindControlRingAnim(MCAnim);
	MCAnim->SetOwnerObject(pTarget);

	return 0x469AA4;
}
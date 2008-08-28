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

	if(pThis->get_MindControl())
	{
		pData->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", pData->MindControl_Permanent);
	}
	if(pThis->get_EMEffect()) {
		pData->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", pData->EMP_Duration);
		pData->EMP_AddDuration = pINI->ReadBool(section, "EMP.AddDuration", pData->EMP_AddDuration);
	}
	/*
	char buffer[256];
	if(pINI->ReadString(section, "IvanBomb.Warhead", pData->Ivan_WH->get_ID(), buffer, 256) > 0)
	{
	pData->Ivan_WH           = WarheadTypeClass::FindOrAllocate(buffer);
	}
	*/
}

// 5240BD, 7 
EXPORT_FUNC(CyborgParsingMyArse) { return 0x5240C4; }

// feature #384: Permanent MindControl Warheads
// 46920B, 6
EXPORT_FUNC(BulletClass_Fire)
{
	GET(BulletClass *, Bullet, ESI);
	WarheadTypeClass *pThis = Bullet->get_WH();

	RET_UNLESS(CONTAINS(WarheadTypeClassExt::Ext_p, pThis));
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[pThis];

	RET_UNLESS(pData->MindControl_Permanent || pData->EMP_Duration);

	TechnoClass *pTarget = (TechnoClass *)Bullet->get_Target();
	RET_UNLESS(pTarget->get_AbstractFlags() & ABSFLAGS_ISTECHNO);

	TechnoTypeClass *pType = (TechnoTypeClass *)pTarget->GetType();

	if(pTarget->IsIronCurtained()) return 0;

	if(pData->MindControl_Permanent) {
		if(pType->get_ImmuneToPsionics()) return 0;
		if(pTarget->get_MindControlledBy()) {
			pTarget->get_MindControlledBy()->get_CaptureManager()->FreeUnit(pTarget);
		}
		pTarget->SetOwningHouse(Bullet->get_Owner()->get_Owner(), 1);
		pTarget->set_MindControlledByAUnit(1);
		pTarget->QueueMission(mission_Guard, 0);

		CoordStruct coords;
		pTarget->GetCoords(&coords);
		coords.Z += pType->get_MindControlRingOffset();

		AnimClass *MCAnim = new AnimClass(RulesClass::Global()->get_PermaControlledAnimationType(), coords);
		pTarget->set_MindControlRingAnim(MCAnim);
		MCAnim->SetOwnerObject(pTarget);

		return 0x469AA4;
	} else if(pData->EMP_Duration && pThis->get_CellSpread() < 0.5) {		
		TechnoClass *pOwner = (TechnoClass *)Bullet->get_Owner();
		int TargetStrength = pTarget->get_Health();

		switch(pTarget->WhatAmI()) {
				case abs_Unit:
					break;
				case abs_Aircraft:
					pTarget->ReceiveDamage(&TargetStrength, 0, RulesClass::Global()->get_C4Warhead(), pOwner, 1, 0, NULL);
					return 0;
				case abs_Infantry:
					RET_UNLESS(pType->get_Cyborg_());
					break;
				default:			
					return 0;
		}

		if(pData->EMP_AddDuration) {
			pTarget->set_EMPLockRemaining(pTarget->get_EMPLockRemaining() + pData->EMP_Duration);
		} else {
			pTarget->set_EMPLockRemaining(pData->EMP_Duration);
		}
		return 0;
	}/* else if(pData->EMP_Duration) {
		int Spread = int(pThis->get_CellSpread());
		int countCells = CellSpread::NumCells(Spread);
		TechnoClass*  curTechno;
		Ares::Log("EMP Warhead with CellSpread detonated, spreading has occured over %d cells.\n", countCells);
		for(int i = 0; i < countCells; ++i) {
			CellStruct tmpCell = CellSpread::GetCell(i);
			CellStruct CenterCell = *MapClass::Global()->GetCellAt(pTarget->get_Location())->get_MapCoords();
			tmpCell.X += CenterCell.X;
			tmpCell.Y += CenterCell.Y;
			CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
			//Ares::Log("Cycling through cell %d. Cell coordinates: %d %d.\n", i, tmpCell.X, tmpCell.Y);
			for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject()) {
				if(!(curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO)) continue;
				curTechno = (TechnoClass *)curObj;
				int TargetStrength = curTechno->get_Health();
				Ares::Log("Found object: %s\n", curTechno->GetType()->get_ID());
				switch(curTechno->WhatAmI()) {
						case abs_Unit:
							break;
						case abs_Aircraft:
							curTechno->ReceiveDamage(&TargetStrength, 0, RulesClass::Global()->get_C4Warhead(), curTechno, 1, 0, NULL);
							continue;							
						case abs_Infantry:
							if(!pType->get_Cyborg_()) continue;
							break;
						default:			
							continue;
				}
				Ares::Log("Applying EMP...\n");
				if(pData->EMP_AddDuration) {
					curTechno->set_EMPLockRemaining(curTechno->get_EMPLockRemaining() + pData->EMP_Duration);
				} else {
					curTechno->set_EMPLockRemaining(pData->EMP_Duration);
				}
				Ares::Log("EMP successful.\n");
			}
		}
		return 0;
	}*/
	return 0;
}
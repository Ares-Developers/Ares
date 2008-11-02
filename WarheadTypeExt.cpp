#include "WarheadTypeExt.h"
#include "ArmorTypes.h"

EXT_P_DEFINE(WarheadTypeClass);

WarheadTypeClass *WarheadTypeClassExt::Temporal_WH = NULL;

void __stdcall WarheadTypeClassExt::Create(WarheadTypeClass* pThis)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Is_Custom = 0;
		pData->Is_Initialized = 0;

		pData->MindControl_Permanent = 0;

		pData->EMP_Duration = 0;

		pData->IC_Duration = 0;

		for(int i = 0; i < 11; ++i)
		{
			pData->Verses.AddItem(1.00);
		}

		pData->Temporal_WarpAway = NULL;

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

void WarheadTypeClassExt::WarheadTypeClassData::Initialize(WarheadTypeClass* pThis)
{
	this->Temporal_WarpAway = RulesClass::Global()->get_WarpAway();
	this->Is_Initialized = 1;
}

void __stdcall WarheadTypeClassExt::LoadFromINI(WarheadTypeClass* pThis, CCINIClass* pINI)
{
	const char * section = pThis->get_ID();

	DEBUGLOG("Reading ext for %s\n", section);

	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	if(!pData->Is_Initialized)
	{
		pData->Initialize(pThis);
	}

	// writing custom verses parser just because
	char buffer[0x100];
	DEBUGLOG("\n[%s]Verses=", section);
	if(pINI->ReadString(section, "Verses", "", buffer, 0x100))
	{
		DEBUGLOG("\t%s", buffer);
		int idx = 0;
		for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ","))
		{
			DEBUGLOG("\n\t\tVerses #%d is %s", idx, cur);
			pData->Verses[idx] = Conversions::Str2Armor(cur);
			DEBUGLOG("\n\t\tWhich converts to %lf", pData->Verses[idx]);
			++idx;
			if(idx > 10)
			{
				break;
			}
		}
	}

	ArmorType::LoadForWarhead(pINI, pThis);

	if(pThis->get_MindControl())
	{
		pData->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", pData->MindControl_Permanent);
		pData->Is_Custom |= pData->MindControl_Permanent;
	}

	if(pThis->get_EMEffect())
	{
		pData->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", pData->EMP_Duration);
		pData->Is_Custom |= 1;
	}

	pData->IC_Duration = pINI->ReadInteger(section, "IronCurtain.Duration", pData->IC_Duration);
	pData->Is_Custom |= !!pData->IC_Duration;

	if(pThis->get_Temporal())
	{
		PARSE_BUF();

		PARSE_ANIM("Temporal.WarpAway", pData->Temporal_WarpAway);
	}
}

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7)
{
	return 0x5240C4;
}

//Feature #200: EMP Warheads
DEFINE_HOOK(4C5824, EMPulseClass_Initialize1, 5)
{
	GET(int, Duration, EDX);
	GET(TechnoClass *, curVictim, ESI);
	Duration += curVictim->get_EMPLockRemaining();
	R->set_EDX(Duration);
	return 0;
}

DEFINE_HOOK(4C5718, EMPulseClass_Initialize2, 6)
{
	GET(int, Duration, EAX);
	GET(TechnoClass *, curVictim, ESI);
	Duration += curVictim->get_EMPLockRemaining();
	R->set_EAX(Duration);
	return 0;
}

DEFINE_HOOK(4C575E, EMPulseClass_CyborgCheck, 7)
{
	GET(TechnoClass *, curVictim, ESI);
	TechnoTypeClass* pType = (TechnoTypeClass*)curVictim->GetType();	
	return pType->get_Cyborg_() ? 0x4C577A : 0;
}

// feature #384: Permanent MindControl Warheads + feature #200: EMP Warheads
DEFINE_HOOK(46920B, BulletClass_Fire, 6)
{
	GET(BulletClass *, Bullet, ESI);
	WarheadTypeClass *pThis = Bullet->get_WH();

	RET_UNLESS(CONTAINS(WarheadTypeClassExt::Ext_p, pThis));
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[pThis];

	RET_UNLESS(pData->Is_Custom);

	CoordStruct coords;
	Bullet->get_Target()->GetCoords(&coords);
	CellStruct cellCoords = *MapClass::Global()->GetCellAt(&coords)->get_MapCoords();

	if(pData->IC_Duration > 0)
	{
		int countCells = CellSpread::NumCells(int(Bullet->get_WH()->get_CellSpread()));
		for(int i = 0; i < countCells; ++i)
		{
			CellStruct tmpCell = CellSpread::GetCell(i);
			tmpCell += cellCoords;
			CellClass *c = MapClass::Global()->GetCellAt(&tmpCell);
			for(ObjectClass *curObj = c->get_FirstObject(); curObj; curObj = curObj->get_NextObject())
			{
				if(curObj->get_AbstractFlags() & ABSFLAGS_ISTECHNO)
				{
					((TechnoClass *)curObj)->IronCurtain(pData->IC_Duration, Bullet->get_Owner()->get_Owner(), 0);
				}
			}
		}
	}

	RET_UNLESS(Bullet->get_Target()->get_AbstractFlags() & ABSFLAGS_ISTECHNO);
	TechnoClass *pTarget = (TechnoClass *)Bullet->get_Target();
	TechnoTypeClass *pType = pTarget->GetTechnoType();

	//Electro@pd: get working on support for multiple callbacks in Syringe
	if(pData->EMP_Duration)
	{
		new EMPulseClass(*(Bullet->get_Target()->GetCell()->get_MapCoords()), int(pThis->get_CellSpread()), pData->EMP_Duration, 0);
	}

	if(pData->MindControl_Permanent)
	{
		if(!pType || pType->get_ImmuneToPsionics()) return 0;
		if(pTarget->get_MindControlledBy())
		{
			pTarget->get_MindControlledBy()->get_CaptureManager()->FreeUnit(pTarget);
		}
		pTarget->SetOwningHouse(Bullet->get_Owner()->get_Owner(), 1);
		pTarget->set_MindControlledByAUnit(1);
		pTarget->QueueMission(mission_Guard, 0);

		coords.Z += pType->get_MindControlRingOffset();

		AnimClass *MCAnim = new AnimClass(RulesClass::Global()->get_PermaControlledAnimationType(), &coords);
		AnimClass *oldMC = pTarget->get_MindControlRingAnim();
		if(oldMC)
		{
			oldMC->UnInit();
		}
		pTarget->set_MindControlRingAnim(MCAnim);
		MCAnim->SetOwnerObject(pTarget);

		return 0x469AA4;
	}

	return 0;
}

#define GET_VERSES(reg_wh, reg_armor) \
		GET(WarheadTypeClass *, WH, reg_wh); \
	GET(int, Armor, reg_armor); \
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[WH]; \
	double x = pData->Verses[Armor];

#define FLD_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	__asm{ fld x }; \
	return R->get_Origin() + 7;

#define FMUL_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	__asm{ fmul x }; \
	return R->get_Origin() + 7;

// temp, will be taken out when SelectWeapon is remade
DEFINE_HOOK(6F36FE, Verses_fld_0, 7)
{
	FLD_VERSES(EAX, ECX);
}

DEFINE_HOOK(6F7D3D, Verses_fld_1, 7)
{
	FLD_VERSES(ECX, EAX);
}

DEFINE_HOOK(708AF7, Verses_fld_2, 7)
{
	FLD_VERSES(ECX, EAX);
}

DEFINE_HOOK(6FCB6A, Verses_fld_3, 7)
{
	FLD_VERSES(EDI, EAX);
}

// temp, will be taken out when SelectWeapon is remade
DEFINE_HOOK(6F3731, Verses_fld_4, 7)
{
	FLD_VERSES(EDX, EAX);
}

DEFINE_HOOK(70CEB2, Verses_fmul_0, 7)
{
	FMUL_VERSES(EAX, ECX);
}

DEFINE_HOOK(70CEC7, Verses_fmul_1, 7)
{
	FMUL_VERSES(EAX, EDX);
}

DEFINE_HOOK(70CF49, Verses_fmul_2, 7)
{
	FMUL_VERSES(ECX, EAX);
}

DEFINE_HOOK(48923D, Verses_fmul_3, 7)
{
	FMUL_VERSES(EDI, EDX);
}

DEFINE_HOOK(75DDCC, Verses_OrigParser, 7)
{
	// should really be doing something smarter due to westwood's weirdass code, but cannot be bothered atm
	// will fix if it is reported to actually break things
	// this breaks 51E33D which stops infantry with verses (heavy=0 & steel=0) from targeting non-infantry at all
	// (whoever wrote that code must have quite a few gears missing in his head)
	return 0x75DE98;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A87B, TemporalClass_Update_CacheWH, 6)
{
	WarheadTypeClassExt::Temporal_WH = ((WeaponTypeClass *)R->get_EAX())->get_Warhead();
	return 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A900, TemporalClass_Update_WarpAway, 6)
{
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[WarheadTypeClassExt::Temporal_WH];

	R->set_EDX((DWORD)pData->Temporal_WarpAway);
	return 0x71A906;
}
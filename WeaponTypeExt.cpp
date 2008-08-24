#include <YRPP.h>
#include "WeaponTypeExt.h"

EXT_P_DECLARE(WeaponTypeClass);
typedef stdext::hash_map<BombClass *, WeaponTypeClassExt::WeaponTypeClassData *> hash_bombExt;
hash_bombExt WeaponTypeClassExt::BombExt;

void __stdcall WeaponTypeClassExt::Create(WeaponTypeClass* pThis)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Beam_IsCustom     = 0;
		pData->Beam_Color        = ColorStruct(255, 255, 255);
		pData->Beam_Duration     = 15;
		pData->Beam_Amplitude    = 40.0;
		pData->Beam_IsLaser      = 0;
		pData->Beam_IsBigLaser   = 0;

		pData->Ivan_IsCustom     = 0;
		pData->Ivan_KillsBridges = 1;
		pData->Ivan_Damage       = RulesClass::Global()->get_IvanDamage();
		pData->Ivan_Duration     = RulesClass::Global()->get_IvanTimedDelay();
		pData->Ivan_TickingSound = RulesClass::Global()->get_BombTickingSound();
		pData->Ivan_AttachSound  = RulesClass::Global()->get_BombAttachSound();
		pData->Ivan_WH           = RulesClass::Global()->get_IvanWarhead();

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
	const char * section = pThis->get_ID();
	Ares::Log("Weapon %s...\n", section);
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	pData->Beam_IsCustom     |= pThis->get_IsRadBeam() ||
		pINI->IsKeySet(section, "Beam.IsLaser") || 	pINI->IsKeySet(section, "Beam.IsBigLaser");
	pData->Beam_IsCustom     &= pINI->ReadBool(section, "Beam.IsCustom", pData->Beam_IsCustom);

	if(pData->Beam_IsCustom)
	{
		ColorStruct tmpColor = pData->Beam_Color;
		pINI->ReadColor(&tmpColor, section, "Beam.Color", &pData->Beam_Color);
		pData->Beam_Color     = tmpColor;
	
		pData->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", pData->Beam_Duration);
		pData->Beam_Amplitude    = pINI->ReadDouble(section, "Beam.Amplitude", pData->Beam_Amplitude);

		pData->Beam_IsLaser      = pINI->ReadBool(section, "Beam.IsLaser", pData->Beam_IsLaser);
		pData->Beam_IsBigLaser   = pINI->ReadBool(section, "Beam.IsBigLaser", pData->Beam_IsBigLaser);
	}

	pData->Ivan_IsCustom = pThis->get_Warhead()->get_IvanBomb();

	Ares::Log("Weapon %s is %d\n", section, pData->Ivan_IsCustom);

	if(pData->Ivan_IsCustom)
	{
		pData->Ivan_KillsBridges = pINI->ReadBool(section, "IvanBomb.DestroysBridges", pData->Ivan_KillsBridges);
		pData->Ivan_Damage       = pINI->ReadInteger(section, "IvanBomb.Damage", pData->Ivan_Damage);
		pData->Ivan_Duration     = pINI->ReadInteger(section, "IvanBomb.TimedDelay", pData->Ivan_Duration);

		char buffer[256];

		if(pINI->ReadString(section, "IvanBomb.TickingSound", "", buffer, 256) > 0)
		{
			pData->Ivan_TickingSound = VocClass::FindIndex(buffer);
		}

		if(pINI->ReadString(section, "IvanBomb.AttachSound", "", buffer, 256) > 0)
		{
			pData->Ivan_AttachSound  = VocClass::FindIndex(buffer);
		}

		if(!pData->Ivan_WH) {
			pData->Ivan_WH           = RulesClass::Global()->get_IvanWarhead();
		}

		if(pINI->ReadString(section, "IvanBomb.Warhead", pData->Ivan_WH->get_ID(), buffer, 256) > 0)
		{
			Ares::Log("Ivan cfg: Damage %d, Delay %d, Warhead %s\n", 
				pData->Ivan_Damage, pData->Ivan_Duration, buffer);
			pData->Ivan_WH           = WarheadTypeClass::FindOrAllocate(buffer);
		}
	}
}

// 6FD79C, 6
// custom RadBeam colors
EXPORT_FUNC(TechnoClass_FireRadBeam)
{
	GET(RadBeam *, Rad, ESI);
	WeaponTypeClass* Source = (WeaponTypeClass *)R->get_StackVar32(0xC);
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->Beam_IsCustom);
	Rad->set_Color(pData->Beam_Color);
	Rad->set_Period(pData->Beam_Duration);
	Rad->set_Amplitude(pData->Beam_Amplitude);
	return 0x6FD7A8;
}

// 438F8F, 6
// custom ivan bomb attachment 1
EXPORT_FUNC(BombListClass_Add1)
{
	Ares::Log("Bomb 1\n");
	GET(BombClass *, Bomb, ESI);

	BulletClass* Bullet = (BulletClass *)R->get_StackVar32(0x0);
	WeaponTypeClass *Source = Bullet->get_WeaponType();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->Ivan_IsCustom);

		Ares::Log("Ivan arm1: Damage %d, Delay %d, Warhead %s\n", 
			pData->Ivan_Damage, pData->Ivan_Duration, pData->Ivan_WH->get_ID());

	WeaponTypeClassExt::BombExt[Bomb] = pData;
	Bomb->set_DetonationFrame(Unsorted::CurrentFrame + pData->Ivan_Duration);
	Bomb->set_TickSound(pData->Ivan_TickingSound);
	return 0;
}

// 438FD1, 5
// custom ivan bomb attachment 2
EXPORT_FUNC(BombListClass_Add2)
{
	Ares::Log("Bomb 2\n");
	GET(BombClass *, Bomb, ESI);
	BulletClass* Bullet = (BulletClass *)R->get_StackVar32(0x0);
	GET(TechnoClass *, Owner, EBP);
	WeaponTypeClass *Source = Bullet->get_WeaponType();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->Ivan_IsCustom && Owner->get_Owner()->ControlledByPlayer());

		Ares::Log("Ivan arm2: Damage %d, Delay %d, Warhead %s\n", 
			pData->Ivan_Damage, pData->Ivan_Duration, pData->Ivan_WH->get_ID());
	
	if(pData->Ivan_AttachSound != -1)
	{
		VocClass::PlayAt(pData->Ivan_AttachSound, Bomb->get_TargetUnit()->get_Location());
	}

	Ares::Log("Bomb 22\n");

	R->set_ESP(R->get_ESP()+0x4);
	Ares::Log("Bomb 23\n");
	R->set_ESI((DWORD)Bullet);
	Ares::Log("Bomb 24\n");
	return 0x439022;
}

// 438799, 6
// custom ivan bomb detonation 1
EXPORT_FUNC(BombClass_Detonate1)
{
	Ares::Log("Detonate 1\n");
	GET(BombClass *, Bomb, ESI);
	
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

	Ares::Log("Ivan det1: Damage %d, Delay %d, Warhead %s\n", 
		pData->Ivan_Damage, pData->Ivan_Duration, pData->Ivan_WH->get_ID());

	R->set_StackVar32(0x8, (DWORD)pData->Ivan_WH);
	R->set_EDX((DWORD)pData->Ivan_Damage);
	return 0x43879F;
}

// 438843, 6
// custom ivan bomb detonation 2
EXPORT_FUNC(BombClass_Detonate2)
{
	Ares::Log("Detonate 2\n");
	GET(BombClass *, Bomb, ESI);
	
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

		Ares::Log("Ivan det2: Damage %d, Delay %d, Warhead %s\n", 
			pData->Ivan_Damage, pData->Ivan_Duration, pData->Ivan_WH->get_ID());

	R->set_EDX((DWORD)pData->Ivan_WH);
	R->set_ECX((DWORD)pData->Ivan_Damage);
	return 0x438849;
}

// 438879, 6
// custom ivan bomb detonation 3
EXPORT_FUNC(BombClass_Detonate3)
{
	Ares::Log("Detonate 3\n");
	GET(BombClass *, Bomb, ESI);

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);
	return pData->Ivan_KillsBridges ? 0x438989 : 0;
}

// 4393F2, 5
// custom ivan bomb cleanup
EXPORT_FUNC(BombClass_SDDTOR)
{
	Ares::Log("Bomb SDDTOR\n");
	GET(BombClass *, Bomb, ECX);
	hash_bombExt::iterator i = WeaponTypeClassExt::BombExt.find(Bomb);
	if(i != WeaponTypeClassExt::BombExt.end())
	{
		WeaponTypeClassExt::BombExt[Bomb] = 0;
		WeaponTypeClassExt::BombExt.erase(Bomb);
	}
	return 0;
}

// custom beam styles
// 6FF43F, 6
EXPORT_FUNC(TechnoClass_Fire)
{
	GET(WeaponTypeClass *, Source, EBX);
	GET(TechnoClass *, Owner, ESI);
	GET(TechnoClass *, Target, EDI);

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];

	if((!pData->Beam_IsLaser && !pData->Beam_IsBigLaser) || Owner->get_Wave())
	{
		return 0;
	}

	DWORD crdS = R->get_EBP();
	crdS += 12;

	CoordStruct crdSrc = *Owner->get_Location();

	CoordStruct crdTgt = *Target->get_Location();

	Ares::Log("Wave coords: (%d, %d, %d) - (%d, %d, %d) \n",
		crdSrc.X, crdSrc.Y, crdSrc.Z, crdTgt.X, crdTgt.Y, crdTgt.Z);

	WaveClass *Wave = new WaveClass(&crdSrc, &crdTgt, 
		Owner, pData->Beam_IsBigLaser ? 2 : 1, Target);
	Owner->set_Wave(Wave);
	return 0x6FF48A;
}

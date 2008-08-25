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
		pData->Ivan_Detachable   = 1;
		pData->Ivan_Damage       = 0;
		pData->Ivan_Delay        = 0;
		pData->Ivan_TickingSound = -1;
		pData->Ivan_AttachSound  = -1;
		pData->Ivan_WH           = NULL;
		pData->Ivan_Image        = NULL;
		pData->Ivan_FlickerRate  = 30;

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
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	if(!pINI->ReadBool(section, "Beam.IsCustom", 1))
	{
		pData->Beam_IsCustom = 0;
	}
	else
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

	if(pData->Ivan_IsCustom)
	{

		if(!pData->Ivan_WH) {
			pData->Ivan_Damage       = RulesClass::Global()->get_IvanDamage();
			pData->Ivan_Delay        = RulesClass::Global()->get_IvanTimedDelay();
			pData->Ivan_TickingSound = RulesClass::Global()->get_BombTickingSound();
			pData->Ivan_AttachSound  = RulesClass::Global()->get_BombAttachSound();
			pData->Ivan_WH           = RulesClass::Global()->get_IvanWarhead();
			pData->Ivan_Image        = RulesClass::Global()->get_BOMBCURS_SHP();
			pData->Ivan_FlickerRate  = RulesClass::Global()->get_IvanIconFlickerRate();
		}

		pData->Ivan_KillsBridges = pINI->ReadBool(section, "IvanBomb.DestroysBridges", pData->Ivan_KillsBridges);
		pData->Ivan_Detachable   = pINI->ReadBool(section, "IvanBomb.Detachable", pData->Ivan_Detachable);

		pData->Ivan_Damage       = pINI->ReadInteger(section, "IvanBomb.Damage", pData->Ivan_Damage);
		pData->Ivan_Delay        = pINI->ReadInteger(section, "IvanBomb.Delay", pData->Ivan_Delay);

		int flicker = pINI->ReadInteger(section, "IvanBomb.FlickerRate", pData->Ivan_FlickerRate);
		if(flicker)
		{
			pData->Ivan_FlickerRate  = flicker;
		}

		char buffer[256];

		if(pINI->ReadString(section, "IvanBomb.TickingSound", "", buffer, 256) > 0)
		{
			pData->Ivan_TickingSound = VocClass::FindIndex(buffer);
		}

		if(pINI->ReadString(section, "IvanBomb.AttachSound", "", buffer, 256) > 0)
		{
			pData->Ivan_AttachSound  = VocClass::FindIndex(buffer);
		}

		if(pINI->ReadString(section, "IvanBomb.Warhead", pData->Ivan_WH->get_ID(), buffer, 256) > 0)
		{
			pData->Ivan_WH           = WarheadTypeClass::FindOrAllocate(buffer);
		}
		
		pINI->ReadString(section, "IvanBomb.Image", "", buffer, 256);
		if(strlen(buffer))
		{
			SHPStruct *image = FileSystem::LoadSHPFile(buffer);
			if(image)
			{
				pData->Ivan_Image        = image;
			}
			else
			{
				Ares::Log("Loading Ivan Image %s failed, reverting to default\n", buffer);
			}
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
	GET(BombClass *, Bomb, ESI);

	BulletClass* Bullet = (BulletClass *)R->get_StackVar32(0x0);
	WeaponTypeClass *Source = Bullet->get_WeaponType();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->Ivan_IsCustom);

	WeaponTypeClassExt::BombExt[Bomb] = pData;
	Bomb->set_DetonationFrame(Unsorted::CurrentFrame + pData->Ivan_Delay);
	Bomb->set_TickSound(pData->Ivan_TickingSound);
	return 0;
}

// 438FD1, 5
// custom ivan bomb attachment 2
EXPORT_FUNC(BombListClass_Add2)
{
	GET(BombClass *, Bomb, ESI);
	BulletClass* Bullet = (BulletClass *)R->get_StackVar32(0x0);
	GET(TechnoClass *, Owner, EBP);
	WeaponTypeClass *Source = Bullet->get_WeaponType();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];
	RET_UNLESS(pData->Ivan_IsCustom && Owner->get_Owner()->ControlledByPlayer());

	if(pData->Ivan_AttachSound != -1)
	{
		VocClass::PlayAt(pData->Ivan_AttachSound, Bomb->get_TargetUnit()->get_Location());
	}

	return 0;
}

// 438FD7, 7
// custom ivan bomb attachment 3
EXPORT_FUNC(BombListClass_Add3)
{
	return 0x439022;
}

// 6F5230, 5
// custom ivan bomb drawing 1
EXPORT_FUNC(TechnoClass_DrawExtras1)
{
	GET(BombClass *, Bomb, ECX);

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

	int frame = 2 * (Unsorted::CurrentFrame - Bomb->get_PlantingFrame()) / (pData->Ivan_Delay / pData->Ivan_Image->Frames);

	if(Unsorted::CurrentFrame % (2 * pData->Ivan_FlickerRate) >= pData->Ivan_FlickerRate)
	{
		++frame;
	}

	if( frame > pData->Ivan_Image->Frames )
	{
		frame = pData->Ivan_Image->Frames;
	}
	else if(frame == pData->Ivan_Image->Frames )
	{
		--frame;
	}

	R->set_EAX(frame);

	return 0x6F5235;
}

// 6F523C, 5
// custom ivan bomb drawing 2
EXPORT_FUNC(TechnoClass_DrawExtras2)
{
	DWORD pBomb = R->get_EBP();
	pBomb += 0x38;
	BombClass * Bomb = (BombClass *)pBomb;

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

	if(!pData->Ivan_Image)
	{
		Ares::Log("No Ivan Image!\n");
		return 0;
	}

	DWORD pImage = (DWORD)pData->Ivan_Image;
	Ares::Log("Ivan Image = %08X\n", pImage);

	R->set_ECX(pImage);
	return 0x6F5247;
}

// 6FCBAD, 6
// custom ivan bomb disarm 1
EXPORT_FUNC(TechnoClass_GetObjectActivityState)
{
	GET(TechnoClass *, Target, EBP);
	BombClass *Bomb = Target->get_AttachedBomb();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);
	if(!pData->Ivan_Detachable)
	{
		return 0x6FCBBE;
	}
	return 0;
}

// 51E488, 5
EXPORT_FUNC(InfantryClass_GetCursorOverObject2)
{
	GET(TechnoClass *, Target, ESI);
	BombClass *Bomb = Target->get_AttachedBomb();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);
	if(!pData->Ivan_Detachable)
	{
		return 0x51E49E;
	}
	return 0;
}

// 438799, 6
// custom ivan bomb detonation 1
EXPORT_FUNC(BombClass_Detonate1)
{
	GET(BombClass *, Bomb, ESI);
	
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

	R->set_StackVar32(0x4, (DWORD)pData->Ivan_WH);
	R->set_EDX((DWORD)pData->Ivan_Damage);
	return 0x43879F;
}

// 438843, 6
// custom ivan bomb detonation 2
EXPORT_FUNC(BombClass_Detonate2)
{
	GET(BombClass *, Bomb, ESI);
	
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
	RET_UNLESS(pData->Ivan_IsCustom);

	R->set_EDX((DWORD)pData->Ivan_WH);
	R->set_ECX((DWORD)pData->Ivan_Damage);
	return 0x438849;
}

// 438879, 6
// custom ivan bomb detonation 3
EXPORT_FUNC(BombClass_Detonate3)
{
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
// 6FF5F5, 6
EXPORT_FUNC(TechnoClass_Fire)
{
	GET(WeaponTypeClass *, Source, EBX);
	GET(TechnoClass *, Owner, ESI);
	GET(TechnoClass *, Target, EDI);

	DWORD pTarget = R->get_EBP();

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];

	if((!pData->Beam_IsLaser && !pData->Beam_IsBigLaser))
	{
		return 0;
	}

	if(Owner->get_Wave())
	{
		Ares::Log("Freeing wave\n");
		Owner->set_Wave(NULL);
	}
	else
	{
		DWORD pESP = R->get_ESP();

		DWORD xyzS = pESP + 0x44;
		DWORD xyzT = pESP + 0x88;

		CoordStruct *xyzSrc = (CoordStruct *)xyzS, *xyzTgt = (CoordStruct *)xyzT; 

		Ares::Log("Wave coords: (%d, %d, %d) - (%d, %d, %d) \n",
			xyzSrc->X, xyzSrc->Y, xyzSrc->Z, xyzTgt->X, xyzTgt->Y, xyzTgt->Z);

		WaveClass *Wave = new WaveClass(xyzSrc, xyzTgt, Owner, pData->Beam_IsBigLaser ? 2 : 1, Target);
		Owner->set_Wave(Wave);
	}
	return 0x6FF650;
//
//	return 0;
}

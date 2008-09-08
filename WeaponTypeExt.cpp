#include <YRPP.h>
#include "WeaponTypeExt.h"

EXT_P_DEFINE(WeaponTypeClass);
typedef stdext::hash_map<BombClass *, WeaponTypeClassExt::WeaponTypeClassData *> hash_bombExt;
typedef stdext::hash_map<WaveClass *, WeaponTypeClassExt::WeaponTypeClassData *> hash_waveExt;

hash_bombExt WeaponTypeClassExt::BombExt;
hash_waveExt WeaponTypeClassExt::WaveExt;

void __stdcall WeaponTypeClassExt::Create(WeaponTypeClass* pThis)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Weapon_Loaded  = 1;
		pData->Is_Initialized = 0;

		pData->Beam_Duration     = 15;
		pData->Beam_Amplitude    = 40.0;
		pData->Beam_Color      = ColorStruct(255, 255, 255);

		pData->Wave_IsLaser      = 0;
		pData->Wave_IsBigLaser   = 0;
		pData->Wave_Color      = ColorStruct(255, 255, 255);
/*
		pData->Wave_InitialIntensity = 160;
		pData->Wave_IntensityStep = -6;
		pData->Wave_FinalIntensity = 32; // yeah, they don't match well. Tell that to WW.
*/

		// these can't be initialized to meaningful values, rules class is not loaded from ini yet
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

void WeaponTypeClassExt::WeaponTypeClassData::Initialize(WeaponTypeClass* pThis)
{
	this->Ivan_Damage       = RulesClass::Global()->get_IvanDamage();
	this->Ivan_Delay        = RulesClass::Global()->get_IvanTimedDelay();
	this->Ivan_TickingSound = RulesClass::Global()->get_BombTickingSound();
	this->Ivan_AttachSound  = RulesClass::Global()->get_BombAttachSound();
	this->Ivan_WH           = RulesClass::Global()->get_IvanWarhead();
	this->Ivan_Image        = RulesClass::Global()->get_BOMBCURS_SHP();
	this->Ivan_FlickerRate  = RulesClass::Global()->get_IvanIconFlickerRate();
	if(pThis->get_IsRadBeam() || pThis->get_IsRadEruption())
	{
		if(pThis->get_Warhead()->get_Temporal())
		{
			this->Beam_Color = *RulesClass::Global()->get_ChronoBeamColor(); // Well, a RadEruption Temporal will look pretty funny
		}
		else
		{
			this->Beam_Color = *RulesClass::Global()->get_RadColor();
		}
	}
	this->Is_Initialized = 1;
}

void __stdcall WeaponTypeClassExt::LoadFromINI(WeaponTypeClass* pThis, CCINIClass* pINI)
{
	const char * section = pThis->get_ID();
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];

	if(!pData->Is_Initialized)
	{
		pData->Initialize(pThis);
	}

	if(pThis->get_Damage() == 0 && pData->Weapon_Loaded)
	{
		// blargh
		// this is the ugly case of a something that apparently isn't loaded from ini yet, wonder why
		pData->Weapon_Loaded = 0;
		pThis->LoadFromINI(pINI);
		return;
	}

	ColorStruct tmpColor;
	PARSE_COLOR("Beam.Color", pData->Beam_Color, tmpColor);

	pData->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", pData->Beam_Duration);
	pData->Beam_Amplitude    = pINI->ReadDouble(section, "Beam.Amplitude", pData->Beam_Amplitude);

	pData->Wave_IsLaser      = pINI->ReadBool(section, "Wave.IsLaser", pData->Wave_IsLaser);
	pData->Wave_IsBigLaser   = pINI->ReadBool(section, "Wave.IsBigLaser", pData->Wave_IsBigLaser);
	
	PARSE_COLOR("Wave.Color", pData->Wave_Color, tmpColor);

/*
	pData->Wave_InitialIntensity = pINI->ReadInteger(section, "Wave.InitialIntensity", pData->Wave_InitialIntensity);
	pData->Wave_IntensityStep    = pINI->ReadInteger(section, "Wave.IntensityStep", pData->Wave_IntensityStep);
	pData->Wave_FinalIntensity   = pINI->ReadInteger(section, "Wave.FinalIntensity", pData->Wave_FinalIntensity);
*/

	if(!pThis->get_Warhead())
	{
		Ares::Log("Weapon %s doesn't have a Warhead yet, what gives?\n", section);
		return;
	}

	if(pThis->get_Warhead()->get_IvanBomb())
	{
		pData->Ivan_KillsBridges = pINI->ReadBool(section, "IvanBomb.DestroysBridges", pData->Ivan_KillsBridges);
		pData->Ivan_Detachable   = pINI->ReadBool(section, "IvanBomb.Detachable", pData->Ivan_Detachable);

		pData->Ivan_Damage       = pINI->ReadInteger(section, "IvanBomb.Damage", pData->Ivan_Damage);
		pData->Ivan_Delay        = pINI->ReadInteger(section, "IvanBomb.Delay", pData->Ivan_Delay);

		int flicker = pINI->ReadInteger(section, "IvanBomb.FlickerRate", pData->Ivan_FlickerRate);
		if(flicker)
		{
			pData->Ivan_FlickerRate  = flicker;
		}

		PARSE_BUF();

		PARSE_SND("IvanBomb.TickingSound", pData->Ivan_TickingSound);

		PARSE_SND("IvanBomb.AttachSound", pData->Ivan_AttachSound);

		PARSE_WH("IvanBomb.Warhead", pData->Ivan_WH);
		
		pINI->ReadString(section, "IvanBomb.Image", "", buffer, 256);
		if(strlen(buffer))
		{
			SHPStruct *image = FileSystem::LoadSHPFile(buffer);
			if(image)
			{
				Ares::Log("Loading Ivan Image %s succeeded: %d frames\n", buffer, image->Frames);
				pData->Ivan_Image        = image;
			}
			else
			{
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
	RET_UNLESS(Owner->get_Owner()->ControlledByPlayer());

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
	GET(TechnoClass *, pThis, EBP);
	BombClass * Bomb = pThis->get_AttachedBomb();

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];

	if(pData->Ivan_Image->Frames < 2)
	{
		R->set_EAX(0);
		return 0x6F5235;
	}

	int frame = 
	(Unsorted::CurrentFrame - Bomb->get_PlantingFrame())
		/ (pData->Ivan_Delay / (pData->Ivan_Image->Frames - 1)); // -1 so that last iteration has room to flicker

	if(Unsorted::CurrentFrame % (2 * pData->Ivan_FlickerRate) >= pData->Ivan_FlickerRate)
	{
		++frame;
	}

	if( frame >= pData->Ivan_Image->Frames )
	{
		frame = pData->Ivan_Image->Frames - 1;
	}
	else if(frame == pData->Ivan_Image->Frames - 1 )
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
	GET(TechnoClass *, pThis, EBP);
	BombClass * Bomb = pThis->get_AttachedBomb();

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];

	if(!pData->Ivan_Image)
	{
		Ares::Log("No Ivan Image!\n");
		return 0;
	}

	DWORD pImage = (DWORD)pData->Ivan_Image;

	R->set_ECX(pImage);
	return 0x6F5247;
}

// 6FCBAD, 6
// custom ivan bomb disarm 1
EXPORT_FUNC(TechnoClass_GetObjectActivityState)
{
	GET(TechnoClass *, Target, EBP);
	BombClass *Bomb = Target->get_AttachedBomb();
	RET_UNLESS(Bomb && CONTAINS(WeaponTypeClassExt::BombExt, Bomb));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::BombExt[Bomb];
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

	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Source));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Source];

	RET_UNLESS(pData->Wave_IsLaser || pData->Wave_IsBigLaser);

	DWORD pESP = R->get_ESP();

	DWORD xyzS = pESP + 0x44;
	DWORD xyzT = pESP + 0x88;

	CoordStruct *xyzSrc = (CoordStruct *)xyzS, *xyzTgt = (CoordStruct *)xyzT;

	WaveClass *Wave = new WaveClass(xyzSrc, xyzTgt, Owner, pData->Wave_IsBigLaser ? 2 : 1, Target);
	WeaponTypeClassExt::WaveExt[Wave] = pData;
	Owner->set_Wave(Wave);
	return 0x6FF650;
}

// 75E963, 6
EXPORT_FUNC(WaveClass_CTOR)
{
	GET(WaveClass *, Wave, ESI);
	DWORD Type = R->get_ECX();
	if(Type == wave_Laser || Type == wave_BigLaser)
	{
		return 0;
	}
	GET(WeaponTypeClass *, Weapon, EBX);
	RET_UNLESS(Weapon);
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::Ext_p, Weapon));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::Ext_p[Weapon];
	WeaponTypeClassExt::WaveExt[Wave] = pData;
	return 0;
}

/*
// 75EB87, 0A // fsdblargh, a single instruction spanning 10 bytes
EXPORT_FUNC(WaveClass_CTOR2)
{
	GET(WaveClass *, Wave, ESI);
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::WaveExt, Wave));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::WaveExt[Wave];
//	Wave->set_WaveIntensity(pData->Wave_InitialIntensity);
	return 0x75EB91;
}
*/

// 763226, 6
EXPORT_FUNC(WaveClass_DTOR)
{
	GET(WaveClass *, Wave, EDI);
	hash_waveExt::iterator i = WeaponTypeClassExt::WaveExt.find(Wave);
	if(i != WeaponTypeClassExt::WaveExt.end())
	{
		WeaponTypeClassExt::WaveExt.erase(i);
	}
	return 0;
}

/*
// 760FFC, 5
// Alt beams update
EXPORT_FUNC(WaveClass_UpdateLaser)
{
	GET(WaveClass *, Wave, ESI);
	Wave->Update_Beam();
	RET_UNLESS(CONTAINS(WeaponTypeClassExt::WaveExt, Wave));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::WaveExt[Wave];
	int intense = Wave->get_WaveIntensity() + pData->Wave_IntensityStep;
	Wave->set_WaveIntensity(intense);
	return intense >= pData->Wave_FinalIntensity ? 0x761016 : 0x76100C;
}
*/

// 760BC2, 6
EXPORT_FUNC(WaveClass_Draw2)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EBP);

	WeaponTypeClassExt::ModifyBeamColor(dest, dest, Wave);

	return 0x760CAF;
}

// 760DE2, 6
EXPORT_FUNC(WaveClass_Draw3)
{
	GET(WaveClass *, Wave, EBX);
	GET(WORD *, dest, EDI);

	WeaponTypeClassExt::ModifyBeamColor(dest, dest, Wave);

	return 0x760ECB;
}

// 75EE57, 7
EXPORT_FUNC(WaveClass_Draw_Sonic)
{
	DWORD pWave = R->get_ESP() - 8;
	WaveClass * Wave = (WaveClass *)pWave;
	DWORD src = R->get_EDI();
	DWORD offs = src + R->get_ECX() * 2;

	WeaponTypeClassExt::ModifyBeamColor((WORD *)offs, (WORD *)src, Wave);

	return 0x75EF1C;
}

// 7601FB, 0B
EXPORT_FUNC(WaveClass_Draw_Magnetron)
{
	DWORD pWave = R->get_ESP() - 4;
	WaveClass * Wave = (WaveClass *)pWave;
	DWORD src = R->get_EBX();
	DWORD offs = src + R->get_ECX() * 2;

	WeaponTypeClassExt::ModifyBeamColor((WORD *)offs, (WORD *)src, Wave);

	return 0x7602D3;
}

void WeaponTypeClassExt::ModifyBeamColor(WORD *src, WORD *dst, WaveClass *Wave)
{
	RETZ_UNLESS(CONTAINS(WeaponTypeClassExt::WaveExt, Wave));
	WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::WaveExt[Wave];

	DWORD intensity;
	if(Wave->get_Type() == wave_Laser || Wave->get_Type() == wave_BigLaser)
	{
		intensity = Wave->get_LaserIntensity();
	}
	else
	{
		intensity = Wave->get_WaveIntensity();
	}

	ColorStruct initial = Drawing::WordColor(*src);

	ColorStruct modified = initial;

// ugly hack to fix byte wraparound problems
#define upcolor(c) \
	int _ ## c = initial. c + (intensity * pData->Wave_Color. c ) / 256; \
	_ ## c = min(_ ## c, 255); \
	modified. c = (BYTE)_ ## c;

	upcolor(R);
	upcolor(G);
	upcolor(B);

	WORD color = Drawing::Color16bit(&modified);

	*dst = color;
}

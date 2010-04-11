#include "Body.h"

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x33333333;
Container<WeaponTypeExt> WeaponTypeExt::ExtMap;

template<> WeaponTypeExt::TT *Container<WeaponTypeExt>::SavingObject = NULL;
template<> IStream *Container<WeaponTypeExt>::SavingStream = NULL;

hash_bombExt WeaponTypeExt::BombExt;
hash_waveExt WeaponTypeExt::WaveExt;
hash_boltExt WeaponTypeExt::BoltExt;
hash_radsiteExt WeaponTypeExt::RadSiteExt;

void WeaponTypeExt::ExtData::Initialize(WeaponTypeClass *pThis)
{
	this->Wave_Reverse[idxVehicle] = pThis->IsMagBeam;

	this->_Initialized = is_Inited;
};

void WeaponTypeExt::ExtData::LoadFromINIFile(WeaponTypeExt::TT *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();
	if(!pINI->GetSection(section)) {
		return;
	}

	if(pThis->IsRadBeam || pThis->IsRadEruption) {
		if(pThis->Warhead->Temporal) {
			// Well, a RadEruption Temporal will look pretty funny, but this is what WW uses
			this->Beam_Color.Bind(&RulesClass::Instance->ChronoBeamColor);
		}
	}

	if(pThis->IsMagBeam) {
		this->Wave_Color.Set(ColorStruct(0xB0, 0, 0xD0)); // rp2 values
	} else if(pThis->IsSonic) {
		this->Wave_Color.Set(ColorStruct(0, 0, 0)); // 0,0,0 is a magic value for "no custom handling"
	} else {
		this->Wave_Color.Set(ColorStruct(255, 255, 255)); // placeholder
	}

	if(pThis->Damage == 0 && this->Weapon_Loaded) {
		// blargh
		// this is the ugly case of a something that apparently isn't loaded from ini yet, wonder why
		this->Weapon_Loaded = 0;
		pThis->LoadFromINI(pINI);
		return;
	}

	ColorStruct tmpColor;

	INI_EX exINI(pINI);

	this->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", this->Beam_Duration);
	this->Beam_Amplitude    = pINI->ReadInteger(section, "Beam.Amplitude", this->Beam_Amplitude);
	this->Beam_IsHouseColor = pINI->ReadBool(section, "Beam.IsHouseColor", this->Beam_IsHouseColor);

	if(!this->Beam_IsHouseColor) {
		this->Beam_Color.Read(&exINI, section, "Beam.Color");
	}

	this->Wave_IsLaser      = pINI->ReadBool(section, "Wave.IsLaser", this->Wave_IsLaser);
	this->Wave_IsBigLaser   = pINI->ReadBool(section, "Wave.IsBigLaser", this->Wave_IsBigLaser);
	this->Wave_IsHouseColor = pINI->ReadBool(section, "Wave.IsHouseColor", this->Wave_IsHouseColor);

	if(this->IsWave(pThis) && !this->Wave_IsHouseColor) {
		this->Wave_Color.Read(&exINI, section, "Wave.Color");
	}

	this->Wave_Reverse[idxVehicle]   =
		pINI->ReadBool(section, "Wave.ReverseAgainstVehicles", this->Wave_Reverse[idxVehicle]);
	this->Wave_Reverse[idxAircraft]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstAircraft", this->Wave_Reverse[idxAircraft]);
	this->Wave_Reverse[idxBuilding] =
		pINI->ReadBool(section, "Wave.ReverseAgainstBuildings", this->Wave_Reverse[idxBuilding]);
	this->Wave_Reverse[idxInfantry]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstInfantry", this->Wave_Reverse[idxInfantry]);
	this->Wave_Reverse[idxOther]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstOthers", this->Wave_Reverse[idxOther]);

	if(pThis->IsElectricBolt) {
		this->Bolt_Color1.Read(&exINI, section, "Bolt.Color1");
		this->Bolt_Color2.Read(&exINI, section, "Bolt.Color2");
		this->Bolt_Color3.Read(&exINI, section, "Bolt.Color3");
	}

//	pData->Wave_InitialIntensity = pINI->ReadInteger(section, "Wave.InitialIntensity", pData->Wave_InitialIntensity);
//	pData->Wave_IntensityStep    = pINI->ReadInteger(section, "Wave.IntensityStep", pData->Wave_IntensityStep);
//	pData->Wave_FinalIntensity   = pINI->ReadInteger(section, "Wave.FinalIntensity", pData->Wave_FinalIntensity);

	if(!pThis->Warhead) {
		DEBUGLOG("Weapon %s doesn't have a Warhead yet, what gives?\n", section);
		return;
	}

	if(pThis->Warhead->IvanBomb) {
		this->Ivan_KillsBridges = pINI->ReadBool(section, "IvanBomb.DestroysBridges", this->Ivan_KillsBridges);
		this->Ivan_Detachable   = pINI->ReadBool(section, "IvanBomb.Detachable", this->Ivan_Detachable);

		this->Ivan_Damage.Read(&exINI, section, "IvanBomb.Damage");
		this->Ivan_Delay.Read(&exINI, section, "IvanBomb.Delay");

		this->Ivan_FlickerRate.Read(&exINI, section, "IvanBomb.FlickerRate");

		this->Ivan_TickingSound.Read(&exINI, section, "IvanBomb.TickingSound");

		this->Ivan_AttachSound.Read(&exINI, section, "IvanBomb.AttachSound");

		this->Ivan_WH.Parse(&exINI, section, "IvanBomb.Warhead");

		this->Ivan_Image.Read(&exINI, section, "IvanBomb.Image");
	}
//
/*
	if(pThis->get_RadLevel()) {
//		pData->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", pData->Beam_Duration);
		if(pINI->ReadString(section, "Radiation.Type", "", buffer, 256)) {
			RadType * rType = RadType::Find(buffer);
			if(!rType) {
				Debug::Log("Weapon [%s] references undeclared RadiationType '%s'!\n", section, buffer);
			}
			pData->Rad_Type = rType;
		}
	}
*/
}

void Container<WeaponTypeExt>::InvalidatePointer(void *ptr) {
	AnnounceInvalidPointerMap(WeaponTypeExt::BombExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::WaveExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::BoltExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::RadSiteExt, ptr);
}

// =============================
// load/save

void Container<WeaponTypeExt>::Save(WeaponTypeClass *pThis, IStream *pStm) {
	WeaponTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;

		DynamicVectorClass<BombClass *> *bombs = BombListClass::Global()->get_Bombs();
		pStm->Write(&bombs->Count, 4, &out);
		for(int ii = 0; ii < bombs->Count; ++ii) {
			BombClass *ptr = bombs->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::BombExt[ptr], 4, &out);
		}

		pStm->Write(&WaveClass::Array->Count, 4, &out);
		for(int ii = 0; ii < WaveClass::Array->Count; ++ii) {
			WaveClass *ptr = WaveClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::WaveExt[ptr], 4, &out);
		}

		pStm->Write(&RadSiteClass::Array->Count, 4, &out);
		for(int ii = 0; ii < RadSiteClass::Array->Count; ++ii) {
			RadSiteClass *ptr = RadSiteClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::RadSiteExt[ptr], 4, &out);
		}

	}
}

void Container<WeaponTypeExt>::Load(WeaponTypeClass *pThis, IStream *pStm) {
	WeaponTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	ULONG out;

	int Count;
	WeaponTypeExt::ExtData* data;

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		BombClass *bomb;
		pStm->Read(&bomb, 4, &out);
		SWIZZLE(bomb);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::BombExt[bomb] = data;
	}

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		WaveClass *wave;
		pStm->Read(&wave, 4, &out);
		SWIZZLE(wave);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::WaveExt[wave] = data;
	}

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		RadSiteClass *rad;
		pStm->Read(&rad, 4, &out);
		SWIZZLE(rad);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::RadSiteExt[rad] = data;
	}

	SWIZZLE(pData->Ivan_WH);
	SWIZZLE(pData->Ivan_Image);
}

// =============================
// container hooks

DEFINE_HOOK(771EE9, WeaponTypeClass_CTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}


DEFINE_HOOK(7730F0, WeaponTypeClass_DTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ECX);

	WeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(772CD0, WeaponTypeClass_SaveLoad_Prefix, 7)
DEFINE_HOOK_AGAIN(772EB0, WeaponTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(WeaponTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<WeaponTypeExt>::SavingObject = pItem;
	Container<WeaponTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(772EA6, WeaponTypeClass_Load_Suffix, 6)
{
	WeaponTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(772F8C, WeaponTypeClass_Save, 5)
{
	WeaponTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(7729B0, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(7729C7, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(7729D6, WeaponTypeClass_LoadFromINI, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

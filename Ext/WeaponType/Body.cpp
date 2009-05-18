#include "Body.h"

const DWORD Extension<WeaponTypeClass>::Canary = 0x33333333;
Container<WeaponTypeExt> WeaponTypeExt::ExtMap;

hash_bombExt WeaponTypeExt::BombExt;
hash_waveExt WeaponTypeExt::WaveExt;
hash_radsiteExt WeaponTypeExt::RadSiteExt;

void WeaponTypeExt::ExtData::InitializeRuled(WeaponTypeClass *pThis)
{
	RulesClass * Rules = RulesClass::Global();
	this->Ivan_Damage       = Rules->IvanDamage;
	this->Ivan_Delay        = Rules->IvanTimedDelay;
	this->Ivan_TickingSound = Rules->BombTickingSound;
	this->Ivan_AttachSound  = Rules->BombAttachSound;
	this->Ivan_WH           = Rules->IvanWarhead;
	this->Ivan_Image        = Rules->BOMBCURS_SHP;
	this->Ivan_FlickerRate  = Rules->IvanIconFlickerRate;
	this->_Initialized = is_Ruled;
}

void WeaponTypeExt::ExtData::Initialize(WeaponTypeClass *pThis)
{
	this->Weapon_Source  = pThis;
	RulesClass * Rules = RulesClass::Global();

	if(pThis->IsRadBeam || pThis->IsRadEruption) {
		if(pThis->Warhead->Temporal) {
			this->Beam_Color = *Rules->get_ChronoBeamColor(); // Well, a RadEruption Temporal will look pretty funny
		} else {
			this->Beam_Color = *Rules->get_RadColor();
		}
	}

	if(pThis->IsMagBeam) {
		this->Wave_Color = ColorStruct(0xB0, 0, 0xD0); // rp2 values
	} else if(pThis->IsSonic) {
		this->Wave_Color = ColorStruct(255, 255, 255); // dunno the actual default
	}

	this->Wave_Reverse[idxVehicle] = pThis->IsMagBeam;

	this->_Initialized = is_Inited;
};

void WeaponTypeExt::ExtData::LoadFromINI(WeaponTypeExt::TT *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();
//	ExtData *pData = ExtMap.Find(pThis);
	if(!pINI->GetSection(section)) {
		return;
	}

	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	if(pThis->Damage == 0 && this->Weapon_Loaded) {
		// blargh
		// this is the ugly case of a something that apparently isn't loaded from ini yet, wonder why
		this->Weapon_Loaded = 0;
		pThis->LoadFromINI(pINI);
		return;
	}

	PARSE_BUF();

	ColorStruct tmpColor;

	this->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", this->Beam_Duration);
	this->Beam_Amplitude    = pINI->ReadDouble(section, "Beam.Amplitude", this->Beam_Amplitude);
	this->Beam_IsHouseColor = pINI->ReadBool(section, "Beam.IsHouseColor", this->Beam_IsHouseColor);
	if(!this->Beam_IsHouseColor) {
		PARSE_COLOR("Beam.Color", this->Beam_Color, tmpColor);
	}

	this->Wave_IsLaser      = pINI->ReadBool(section, "Wave.IsLaser", this->Wave_IsLaser);
	this->Wave_IsBigLaser   = pINI->ReadBool(section, "Wave.IsBigLaser", this->Wave_IsBigLaser);
	this->Wave_IsHouseColor = pINI->ReadBool(section, "Wave.IsHouseColor", this->Wave_IsHouseColor);
	if(!this->Wave_IsHouseColor) {
		PARSE_COLOR("Wave.Color", this->Wave_Color, tmpColor);
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

		this->Ivan_Damage       = pINI->ReadInteger(section, "IvanBomb.Damage", this->Ivan_Damage);
		this->Ivan_Delay        = pINI->ReadInteger(section, "IvanBomb.Delay", this->Ivan_Delay);

		int flicker = pINI->ReadInteger(section, "IvanBomb.FlickerRate", this->Ivan_FlickerRate);
		if(flicker)
		{
			this->Ivan_FlickerRate  = flicker;
		}

		PARSE_SND("IvanBomb.TickingSound", this->Ivan_TickingSound);

		PARSE_SND("IvanBomb.AttachSound", this->Ivan_AttachSound);

		PARSE_WH("IvanBomb.Warhead", this->Ivan_WH);
		
		pINI->ReadString(section, "IvanBomb.Image", "", buffer, 256);
		if(strlen(buffer)) {
			SHPStruct *image = FileSystem::LoadSHPFile(buffer);
			if(image) {
				this->Ivan_Image        = image;
			}
		}
	}

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

void WeaponTypeExt::PointerGotInvalid(void *ptr) {

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

DEFINE_HOOK(772EA6, WeaponTypeClass_Load, 6)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x1C);
	GET_STACK(IStream*, pStm, 0x20);

	WeaponTypeExt::ExtMap.Load(pItem, pStm);
	return 0;
}


DEFINE_HOOK(772F8C, WeaponTypeClass_Save, 5)
{
	GET_STACK(WeaponTypeClass*, pItem, 0xC);
	GET_STACK(IStream*, pStm, 0x10);

	WeaponTypeExt::ExtMap.Save(pItem, pStm);
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

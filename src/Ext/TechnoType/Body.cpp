#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Side/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"

#include <AnimTypeClass.h>
#include <PCX.h>
#include <Theater.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x44444444;
Container<TechnoTypeExt> TechnoTypeExt::ExtMap;

template<> TechnoTypeExt::TT *Container<TechnoTypeExt>::SavingObject = NULL;
template<> IStream *Container<TechnoTypeExt>::SavingStream = NULL;

// =============================
// member funcs

void TechnoTypeExt::ExtData::Initialize(TechnoTypeClass *pThis) {
	this->Survivors_PilotChance.SetAll(-1); // was int(RulesClass::Instance->CrewEscape * 100), now negative values indicate "use CrewEscape"
	this->Survivors_PassengerChance.SetAll(-1); // was (int)RulesClass::Global()->CrewEscape * 100); - changed to -1 to indicate "100% if this is a land transport"

	this->Survivors_PilotCount = -1; // defaults to (crew ? 1 : 0)

	this->PrerequisiteLists.SetCapacity(0, NULL);
	this->PrerequisiteLists.AddItem(new DynamicVectorClass<int>);

	this->PrerequisiteTheaters = 0xFFFFFFFF;

	this->Secret_RequiredHouses = 0xFFFFFFFF;
	this->Secret_ForbiddenHouses = 0;

	this->Is_Deso = this->Is_Deso_Radiation = !strcmp(pThis->ID, "DESO");
	this->Is_Cow = !strcmp(pThis->ID, "COW");

	if(pThis->WhatAmI() == AircraftTypeClass::AbsID) {
		this->CustomMissileTrailerAnim = AnimTypeClass::Find("V3TRAIL");
		this->CustomMissileTakeoffAnim = AnimTypeClass::Find("V3TAKOFF");
	}
}

/*
EXT_LOAD(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);

		Ext_p[pThis]->Survivors_Pilots.Load(pStm);
		Ext_p[pThis]->Weapons.Load(pStm);
		Ext_p[pThis]->EliteWeapons.Load(pStm);
		for ( int ii = 0; ii < Ext_p[pThis]->Weapons.get_Count(); ++ii )
			SWIZZLE(Ext_p[pThis]->Weapons[ii].WeaponType);
		for ( int ii = 0; ii < Ext_p[pThis]->EliteWeapons.get_Count(); ++ii )
			SWIZZLE(Ext_p[pThis]->EliteWeapons[ii].WeaponType);
		SWIZZLE(Ext_p[pThis]->Insignia_R);
		SWIZZLE(Ext_p[pThis]->Insignia_V);
		SWIZZLE(Ext_p[pThis]->Insignia_E);
	}
}

EXT_SAVE(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);

		Ext_p[pThis]->Survivors_Pilots.Save(pStm);
		Ext_p[pThis]->Weapons.Save(pStm);
		Ext_p[pThis]->EliteWeapons.Save(pStm);
	}
}
*/

void TechnoTypeExt::ExtData::LoadFromINIFile(TechnoTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	// survivors
	this->Survivors_Pilots.SetCapacity(SideClass::Array->Count, NULL);
	for(int i=this->Survivors_Pilots.Count; i<SideClass::Array->Count; ++i) {
		this->Survivors_Pilots[i] = NULL;
	}
	this->Survivors_Pilots.Count = SideClass::Array->Count;

	this->Survivors_PilotCount = pINI->ReadInteger(section, "Survivor.Pilots", this->Survivors_PilotCount);

	this->Survivors_PilotChance.Read(pINI, section, "Survivor.%sPilotChance");
	this->Survivors_PassengerChance.Read(pINI, section, "Survivor.%sPassengerChance");

	char flag[256];
	for(int i = 0; i < SideClass::Array->Count; ++i) {
		_snprintf(flag, 256, "Survivor.Side%d", i);
		if(pINI->ReadString(section, flag, "", Ares::readBuffer, Ares::readLength)) {
			if((this->Survivors_Pilots[i] = InfantryTypeClass::Find(Ares::readBuffer)) == NULL) {
				if(!INIClass::IsBlank(Ares::readBuffer)) {
					Debug::INIParseFailed(section, flag, Ares::readBuffer);
				}
			}
		}
	}

	// prereqs
	int PrereqListLen = pINI->ReadInteger(section, "Prerequisite.Lists", this->PrerequisiteLists.Count - 1);

	if(PrereqListLen < 1) {
		PrereqListLen = 0;
	}
	++PrereqListLen;
	while(PrereqListLen > this->PrerequisiteLists.Count) {
		this->PrerequisiteLists.AddItem(new DynamicVectorClass<int>);
	}
	while(PrereqListLen < this->PrerequisiteLists.Count) {
		int index = this->PrerequisiteLists.Count - 1;
		if(auto list = this->PrerequisiteLists.GetItem(index)) {
			delete list;
		}
		this->PrerequisiteLists.RemoveItem(index);
	}

	DynamicVectorClass<int> *dvc = this->PrerequisiteLists.GetItem(0);
	Prereqs::Parse(pINI, section, "Prerequisite", dvc);

	dvc = &pThis->PrerequisiteOverride;
	Prereqs::Parse(pINI, section, "PrerequisiteOverride", dvc);

	for(int i = 0; i < this->PrerequisiteLists.Count; ++i) {
		_snprintf(flag, 256, "Prerequisite.List%d", i);
		dvc = this->PrerequisiteLists.GetItem(i);
		Prereqs::Parse(pINI, section, flag, dvc);
	}

	dvc = &this->PrerequisiteNegatives;
	Prereqs::Parse(pINI, section, "Prerequisite.Negative", dvc);

	if(pINI->ReadString(section, "Prerequisite.RequiredTheaters", "", Ares::readBuffer, Ares::readLength)) {
		this->PrerequisiteTheaters = 0;
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			signed int idx = Theater::FindIndex(cur);
			if(idx != -1) {
				this->PrerequisiteTheaters |= (1 << idx);
			} else {
				Debug::INIParseFailed(section, "Prerequisite.RequiredTheaters", cur);
			}
		}
	}

	// new secret lab
	this->Secret_RequiredHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.RequiredHouses", this->Secret_RequiredHouses);

	this->Secret_ForbiddenHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.ForbiddenHouses", this->Secret_ForbiddenHouses);

	this->Is_Deso = pINI->ReadBool(section, "IsDesolator", this->Is_Deso);
	this->Is_Deso_Radiation = pINI->ReadBool(section, "IsDesolator.RadDependant", this->Is_Deso_Radiation);
	this->Is_Cow  = pINI->ReadBool(section, "IsCow", this->Is_Cow);

	this->Is_Spotlighted = pINI->ReadBool(section, "HasSpotlight", this->Is_Spotlighted);
	this->Spot_Height = pINI->ReadInteger(section, "Spotlight.StartHeight", this->Spot_Height);
	this->Spot_Distance = pINI->ReadInteger(section, "Spotlight.Distance", this->Spot_Distance);
	if(pINI->ReadString(section, "Spotlight.AttachedTo", "", Ares::readBuffer, Ares::readLength)) {
		if(!_strcmpi(Ares::readBuffer, "body")) {
			this->Spot_AttachedTo = sa_Body;
		} else if(!_strcmpi(Ares::readBuffer, "turret")) {
			this->Spot_AttachedTo = sa_Turret;
		} else if(!_strcmpi(Ares::readBuffer, "barrel")) {
			this->Spot_AttachedTo = sa_Barrel;
		} else {
			Debug::INIParseFailed(section, "Spotlight.AttachedTo", Ares::readBuffer);
		}
	}
	this->Spot_DisableR = pINI->ReadBool(section, "Spotlight.DisableRed", this->Spot_DisableR);
	this->Spot_DisableG = pINI->ReadBool(section, "Spotlight.DisableGreen", this->Spot_DisableG);
	this->Spot_DisableB = pINI->ReadBool(section, "Spotlight.DisableBlue", this->Spot_DisableB);
	this->Spot_Reverse = pINI->ReadBool(section, "Spotlight.IsInverted", this->Spot_Reverse);

	this->Is_Bomb = pINI->ReadBool(section, "IsBomb", this->Is_Bomb);

/*
	this is too late - Art files are loaded before this hook fires... brilliant
	if(pINI->ReadString(section, "WaterVoxel", "", buffer, 256)) {
		this->WaterAlt = 1;
	}
*/

	INI_EX exINI(pINI);
	this->Insignia.Read(pINI, section, "Insignia.%s");
	this->Parachute_Anim.Parse(&exINI, section, "Parachute.Anim");

	// new on 08.11.09 for #342 (Operator=)
	if(pINI->ReadString(section, "Operator", "", Ares::readBuffer, Ares::readLength)) { // try to read the flag
		this->IsAPromiscuousWhoreAndLetsAnyoneRideIt = (strcmp(Ares::readBuffer, "_ANY_") == 0); // set whether this type accepts all operators
		if(!this->IsAPromiscuousWhoreAndLetsAnyoneRideIt) { // if not, find the specific operator it allows
			if(auto Operator = InfantryTypeClass::Find(Ares::readBuffer)) {
				this->Operator = Operator;
			} else if(!INIClass::IsBlank(Ares::readBuffer)) {
				Debug::INIParseFailed(section, "Operator", Ares::readBuffer);
			}
		}
	}

	this->CameoPal.LoadFromINI(CCINIClass::INI_Art, pThis->ImageFile, "CameoPalette");

	if(pINI->ReadString(section, "Prerequisite.StolenTechs", "", Ares::readBuffer, Ares::readLength)) {
		this->RequiredStolenTech.reset();
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			signed int idx = atoi(cur);
			if(idx > -1 && idx < 32) {
				this->RequiredStolenTech.set(idx);
			} else if(idx != -1) {
				Debug::INIParseFailed(section, "Prerequisite.StolenTechs", cur, "Expected a number between 0 and 31 inclusive");
			}
		}
	}

	this->ImmuneToEMP.Read(&exINI, section, "ImmuneToEMP");
	this->EMP_Modifier = (float)pINI->ReadDouble(section, "EMP.Modifier", this->EMP_Modifier);

	if(pINI->ReadString(section, "EMP.Threshold", "inair", Ares::readBuffer, Ares::readLength)) {
		if(_strcmpi(Ares::readBuffer, "inair") == 0) {
			this->EMP_Threshold = -1;
		} else if((_strcmpi(Ares::readBuffer, "yes") == 0) || (_strcmpi(Ares::readBuffer, "true") == 0)) {
			this->EMP_Threshold = 1;
		} else if((_strcmpi(Ares::readBuffer, "no") == 0) || (_strcmpi(Ares::readBuffer, "false") == 0)) {
			this->EMP_Threshold = 0;
		} else {
			this->EMP_Threshold = pINI->ReadInteger(section, "EMP.Threshold", this->EMP_Threshold);
		}
	}

	if(pINI->ReadString(section, "VeteranAbilities", "", Ares::readBuffer, Ares::readLength)) {
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			if(!_strcmpi(cur, "empimmune")) {
				this->VeteranAbilityEMPIMMUNE = true;
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	if(pINI->ReadString(section, "EliteAbilities", "", Ares::readBuffer, Ares::readLength)) {
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			if(!_strcmpi(cur, "empimmune")) {
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	// #733
	this->ProtectedDriver = pINI->ReadBool(section, "ProtectedDriver", this->ProtectedDriver);
	this->CanDrive = pINI->ReadBool(section, "CanDrive", this->CanDrive);

	// #346, #464, #970, #1014
	this->PassengersGainExperience = pINI->ReadBool(section, "Experience.PromotePassengers", this->PassengersGainExperience);
	this->ExperienceFromPassengers = pINI->ReadBool(section, "Experience.FromPassengers", this->ExperienceFromPassengers);
	this->PassengerExperienceModifier = (float)pINI->ReadDouble(section, "Experience.PassengerModifier", this->PassengerExperienceModifier);
	this->MindControlExperienceSelfModifier = (float)pINI->ReadDouble(section, "Experience.MindControlSelfModifier", this->MindControlExperienceSelfModifier);
	this->MindControlExperienceVictimModifier = (float)pINI->ReadDouble(section, "Experience.MindControlVictimModifier", this->MindControlExperienceVictimModifier);
	this->ExperienceFromAirstrike = pINI->ReadBool(section, "Experience.FromAirstrike", this->ExperienceFromAirstrike);
	this->AirstrikeExperienceModifier = (float)pINI->ReadDouble(section, "Experience.AirstrikeModifier", this->AirstrikeExperienceModifier);

	this->VoiceRepair.Read(&exINI, section, "VoiceIFVRepair");

	this->HijackerEnterSound.Read(&exINI, section, "VehicleThief.EnterSound");
	this->HijackerLeaveSound.Read(&exINI, section, "VehicleThief.LeaveSound");
	this->HijackerKillPilots.Read(&exINI, section, "VehicleThief.KillPilots");
	this->HijackerBreakMindControl.Read(&exINI, section, "VehicleThief.BreakMindControl");
	this->HijackerAllowed.Read(&exINI, section, "VehicleThief.Allowed");
	this->HijackerOneTime.Read(&exINI, section, "VehicleThief.OneTime");

	this->IC_Modifier = (float)pINI->ReadDouble(section, "IronCurtain.Modifier", this->IC_Modifier);

	this->Chronoshift_Allow.Read(&exINI, section, "Chronoshift.Allow");
	this->Chronoshift_IsVehicle.Read(&exINI, section, "Chronoshift.IsVehicle");

	if(CCINIClass::INI_Art->ReadString(pThis->ImageFile, "CameoPCX", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->CameoPCX, Ares::readBuffer, 0x20);
		_strlwr_s(this->CameoPCX, 0x20);
		if(!PCX::Instance->LoadFile(this->CameoPCX)) {
			Debug::INIParseFailed(pThis->ImageFile, "CameoPCX", this->CameoPCX);
		}
	}

	if(CCINIClass::INI_Art->ReadString(pThis->ImageFile, "AltCameoPCX", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->AltCameoPCX, Ares::readBuffer, 0x20);
		_strlwr_s(this->AltCameoPCX, 0x20);
		if(!PCX::Instance->LoadFile(this->AltCameoPCX)) {
			Debug::INIParseFailed(pThis->ImageFile, "AltCameoPCX", this->AltCameoPCX);
		}
	}

	this->CanBeReversed.Read(&exINI, section, "CanBeReversed");

	// #305
	this->RadarJamRadius.Read(&exINI, section, "RadarJamRadius");

	// #1208
	this->PassengerTurret.Read(&exINI, section, "PassengerTurret");
	
	// #617 powered units
	this->PoweredBy.Read(&exINI, section, "PoweredBy");

	this->BuiltAt.Read(&exINI, section, "BuiltAt");

	this->Cloneable.Read(&exINI, section, "Cloneable");

	this->ClonedAt.Read(&exINI, section, "ClonedAt");

	this->CarryallAllowed.Read(&exINI, section, "Carryall.Allowed");
	this->CarryallSizeLimit.Read(&exINI, section, "Carryall.SizeLimit");

	// #680, 1362
	this->ImmuneToAbduction.Read(&exINI, section, "ImmuneToAbduction");

	// issue #896235: cyclic gattling
	this->GattlingCyclic.Read(&exINI, section, "Gattling.Cycle");

	// #245 custom missiles
	if(auto pAircraftType = specific_cast<AircraftTypeClass*>(pThis)) {
		this->IsCustomMissile.Read(&exINI, section, "Missile.Custom");
		this->CustomMissileData.Read(&exINI, section, "Missile");
		this->CustomMissileData.GetEx()->Type = pAircraftType;
		this->CustomMissileWarhead.Parse(&exINI, section, "Missile.Warhead");
		this->CustomMissileEliteWarhead.Parse(&exINI, section, "Missile.EliteWarhead");
		this->CustomMissileTakeoffAnim.Parse(&exINI, section, "Missile.TakeOffAnim");
		this->CustomMissileTrailerAnim.Parse(&exINI, section, "Missile.TrailerAnim");
		this->CustomMissileTrailerSeparation.Read(&exINI, section, "Missile.TrailerSeparation");
	}

	// non-crashable aircraft
	this->Crashable.Read(&exINI, section, "Crashable");

	// quick fix - remove after the rest of weapon selector code is done
	return;
}

/*
	// weapons
	int WeaponCount = pINI->ReadInteger(section, "WeaponCount", pData->Weapons.get_Count());

	if(WeaponCount < 2)
	{
		WeaponCount = 2;
	}

	while(WeaponCount < pData->Weapons.get_Count())
	{
		pData->Weapons.RemoveItem(pData->Weapons.get_Count() - 1);
	}
	if(WeaponCount > pData->Weapons.get_Count())
	{
		pData->Weapons.SetCapacity(WeaponCount, NULL);
		pData->Weapons.set_Count(WeaponCount);
	}

	while(WeaponCount < pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.RemoveItem(pData->EliteWeapons.get_Count() - 1);
	}
	if(WeaponCount > pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.SetCapacity(WeaponCount, NULL);
		pData->EliteWeapons.set_Count(WeaponCount);
	}

	WeaponStruct *W = &pData->Weapons[0];
	ReadWeapon(W, "Primary", section, pINI);

	W = &pData->EliteWeapons[0];
	ReadWeapon(W, "ElitePrimary", section, pINI);

	W = &pData->Weapons[1];
	ReadWeapon(W, "Secondary", section, pINI);

	W = &pData->EliteWeapons[1];
	ReadWeapon(W, "EliteSecondary", section, pINI);

	for(int i = 0; i < WeaponCount; ++i)
	{
		W = &pData->Weapons[i];
		_snprintf(flag, 256, "Weapon%d", i);
		ReadWeapon(W, flag, section, pINI);

		W = &pData->EliteWeapons[i];
		_snprintf(flag, 256, "EliteWeapon%d", i);
		ReadWeapon(W, flag, section, pINI);
	}

void TechnoTypeClassExt::ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI)
{
	char buffer[256];
	char flag[64];

	pINI->ReadString(section, prefix, "", buffer, 0x100);

	if(strlen(buffer))
	{
		pWeapon->WeaponType = WeaponTypeClass::FindOrAllocate(buffer);
	}

	CCINIClass *pArtINI = CCINIClass::INI_Art;

	CoordStruct FLH;
	// (Elite?)(Primary|Secondary)FireFLH - FIRE suffix
	// (Elite?)(Weapon%d)FLH - no suffix
	if(prefix[0] == 'W' || prefix[5] == 'W') // W EliteW
	{
		_snprintf(flag, 64, "%sFLH", prefix);
	}
	else
	{
		_snprintf(flag, 64, "%sFireFLH", prefix);
	}
	pArtINI->Read3Integers((int *)&FLH, section, flag, (int *)&pWeapon->FLH);
	pWeapon->FLH = FLH;

	_snprintf(flag, 64, "%sBarrelLength", prefix);
	pWeapon->BarrelLength = pArtINI->ReadInteger(section, flag, pWeapon->BarrelLength);
	_snprintf(flag, 64, "%sBarrelThickness", prefix);
	pWeapon->BarrelThickness = pArtINI->ReadInteger(section, flag, pWeapon->BarrelThickness);
	_snprintf(flag, 64, "%sTurretLocked", prefix);
	pWeapon->TurretLocked = pArtINI->ReadBool(section, flag, pWeapon->TurretLocked);
}
*/

void Container<TechnoTypeExt>::InvalidatePointer(void *ptr) {
}

bool TechnoTypeExt::ExtData::CameoIsElite()
{
	HouseClass * House = HouseClass::Player;
	HouseTypeClass *Country = House->Type;

	TechnoTypeClass * const T = this->AttachedToObject;
	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(T);

	if(!T->AltCameo && !*pExt->AltCameoPCX) {
		return false;
	}

	switch(T->WhatAmI()) {
		case abs_InfantryType:
			if(House->BarracksInfiltrated && !T->Naval && T->Trainable) {
				return true;
			} else {
				return Country->VeteranInfantry.FindItemIndex((InfantryTypeClass **)&T) != -1;
			}
		case abs_UnitType:
			if(House->WarFactoryInfiltrated && !T->Naval && T->Trainable) {
				return true;
			} else {
				return Country->VeteranUnits.FindItemIndex((UnitTypeClass **)&T) != -1;
			}
		case abs_AircraftType:
			return Country->VeteranAircraft.FindItemIndex((AircraftTypeClass **)&T) != -1;
		case abs_BuildingType:
			if(TechnoTypeClass *Item = T->UndeploysInto) {
				return Country->VeteranUnits.FindItemIndex((UnitTypeClass **)&Item) != -1;
			}
	}

	return false;
}

bool TechnoTypeExt::ExtData::CanBeBuiltAt(BuildingTypeClass * FactoryType) {
	auto pBExt = BuildingTypeExt::ExtMap.Find(FactoryType);
	return (!this->BuiltAt.size() && !pBExt->Factory_ExplicitOnly) || this->BuiltAt.Contains(FactoryType);
}

bool TechnoTypeExt::ExtData::CarryallCanLift(UnitClass * Target) {
	if(Target->ParasiteEatingMe) {
		return false;
	}
	auto TargetData = TechnoTypeExt::ExtMap.Find(Target->Type);
	UnitTypeClass *TargetType = Target->Type;
	bool canCarry = !TargetType->Organic && !TargetType->NonVehicle;
	if(TargetData->CarryallAllowed.isset()) {
		canCarry = !!TargetData->CarryallAllowed;
	}
	if(!canCarry) {
		return false;
	}
	if(this->CarryallSizeLimit.isset()) {
		int maxSize = this->CarryallSizeLimit;
		if(maxSize != -1) {
			return maxSize >= static_cast<TechnoTypeClass *>(Target->Type)->Size;
		}
	}
	return true;

}

// =============================
// load/save

void Container<TechnoTypeExt>::Save(TechnoTypeClass *pThis, IStream *pStm) {
	TechnoTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
		pData->Survivors_Pilots.Save(pStm);

		pData->PrerequisiteLists.Save(pStm);

		Debug::Log("Saving [%s] with %d PreqLists\n", pThis->get_ID(), pData->PrerequisiteLists.Count);
		for(int ii = 0; ii < pData->PrerequisiteLists.Count; ++ii) {
			pData->PrerequisiteLists.Items[ii]->Save(pStm);
		}

		pData->PrerequisiteNegatives.Save(pStm);
		pData->Weapons.Save(pStm);
		pData->EliteWeapons.Save(pStm);
	}
}

void Container<TechnoTypeExt>::Load(TechnoTypeClass *pThis, IStream *pStm) {
	TechnoTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	//ULONG out;

	pData->Survivors_Pilots.Load(pStm, 1);

	pData->PrerequisiteLists.Load(pStm, 1);

	for(int ii = 0; ii < pData->PrerequisiteLists.Count; ++ii) {
		DynamicVectorClass<int> *vec = new DynamicVectorClass<int>();
		vec->Load(pStm, 0);
		pData->PrerequisiteLists.Items[ii] = vec;
	}

	pData->PrerequisiteNegatives.Load(pStm, 0);
	pData->Weapons.Load(pStm, 1);
	pData->EliteWeapons.Load(pStm, 1);

/*
	SWIZZLE(pData->Parachute_Anim);
	SWIZZLE(pData->Insignia_R);
	SWIZZLE(pData->Insignia_V);
	SWIZZLE(pData->Insignia_E);
*/

	for(int ii = 0; ii < pData->Weapons.Count; ++ii) {
		SWIZZLE(pData->Weapons.Items[ii].WeaponType);
	}

	for(int ii = 0; ii < pData->EliteWeapons.Count; ++ii) {
		SWIZZLE(pData->EliteWeapons.Items[ii].WeaponType);
	}
}

// =============================
// container hooks

DEFINE_HOOK(711835, TechnoTypeClass_CTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(711AE0, TechnoTypeClass_DTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
{
	GET_STACK(TechnoTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TechnoTypeExt>::SavingObject = pItem;
	Container<TechnoTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(716DAC, TechnoTypeClass_Load_Suffix, A)
{
	TechnoTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(717094, TechnoTypeClass_Save_Suffix, 5)
{
	TechnoTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

DEFINE_HOOK(679CAF, RulesClass_LoadAfterTypeData_CheckRubbleFoundation, 5) {
	//GET(CCINIClass*, pINI, ESI);

	for(int i=0; i<BuildingTypeClass::Array->Count; ++i) {
		BuildingTypeClass* pTBld = BuildingTypeClass::Array->GetItem(i);
		if(BuildingTypeExt::ExtData *pData = BuildingTypeExt::ExtMap.Find(pTBld)) {
			pData->CompleteInitialization(pTBld);
		}
	}

	return 0;
}


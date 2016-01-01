#include "Body.h"
#include "../BuildingType/Body.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"
#include "../../Utilities/TemplateDef.h"

#include <AbstractClass.h>
#include <HouseClass.h>
#include <PCX.h>
#include <Theater.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x44444444;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

// =============================
// member funcs

void TechnoTypeExt::ExtData::Initialize() {
	auto pThis = this->OwnerObject();

	this->PrerequisiteLists.resize(1);

	this->Is_Deso = this->Is_Deso_Radiation = !strcmp(pThis->ID, "DESO");
	this->Is_Cow = !strcmp(pThis->ID, "COW");

	if(pThis->WhatAmI() == AircraftTypeClass::AbsID) {
		this->CustomMissileTrailerAnim = AnimTypeClass::Find("V3TRAIL");
		this->CustomMissileTakeoffAnim = AnimTypeClass::Find("V3TAKOFF");

		this->SmokeAnim = AnimTypeClass::Find("SGRYSMK1");
	}

	this->EVA_UnitLost = VoxClass::FindIndex("EVA_UnitLost");
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

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char * section = pThis->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	// survivors
	this->Survivors_Pilots.Reserve(SideClass::Array->Count);
	for(int i=this->Survivors_Pilots.Count; i<SideClass::Array->Count; ++i) {
		this->Survivors_Pilots[i] = nullptr;
	}
	this->Survivors_Pilots.Count = SideClass::Array->Count;

	this->Survivors_PilotCount = pINI->ReadInteger(section, "Survivor.Pilots", this->Survivors_PilotCount);

	this->Survivors_PilotChance.Read(exINI, section, "Survivor.%sPilotChance");
	this->Survivors_PassengerChance.Read(exINI, section, "Survivor.%sPassengerChance");

	char flag[256];
	for(int i = 0; i < SideClass::Array->Count; ++i) {
		_snprintf_s(flag, 255, "Survivor.Side%d", i);
		if(pINI->ReadString(section, flag, "", Ares::readBuffer)) {
			if((this->Survivors_Pilots[i] = InfantryTypeClass::Find(Ares::readBuffer)) == nullptr) {
				if(!INIClass::IsBlank(Ares::readBuffer)) {
					Debug::INIParseFailed(section, flag, Ares::readBuffer);
				}
			}
		}
	}

	// prereqs

	// subtract the default list, get tag (not less than 0), add one back
	auto const prerequisiteLists = static_cast<size_t>(
		Math::max(pINI->ReadInteger(section, "Prerequisite.Lists",
		static_cast<int>(this->PrerequisiteLists.size()) - 1), 0) + 1);

	this->PrerequisiteLists.resize(prerequisiteLists);

	Prereqs::Parse(pINI, section, "Prerequisite", this->PrerequisiteLists[0]);

	Prereqs::Parse(pINI, section, "PrerequisiteOverride", pThis->PrerequisiteOverride);

	for(auto i = 0u; i < this->PrerequisiteLists.size(); ++i) {
		_snprintf_s(flag, 255, "Prerequisite.List%u", i);
		Prereqs::Parse(pINI, section, flag, this->PrerequisiteLists[i]);
	}

	Prereqs::Parse(pINI, section, "Prerequisite.Negative", this->PrerequisiteNegatives);

	if(pINI->ReadString(section, "Prerequisite.RequiredTheaters", "", Ares::readBuffer)) {
		this->PrerequisiteTheaters = 0;

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
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
	if(pINI->ReadString(section, "Spotlight.AttachedTo", "", Ares::readBuffer)) {
		if(!_strcmpi(Ares::readBuffer, "body")) {
			this->Spot_AttachedTo = SpotlightAttachment::Body;
		} else if(!_strcmpi(Ares::readBuffer, "turret")) {
			this->Spot_AttachedTo = SpotlightAttachment::Turret;
		} else if(!_strcmpi(Ares::readBuffer, "barrel")) {
			this->Spot_AttachedTo = SpotlightAttachment::Barrel;
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

	this->Insignia.Read(exINI, section, "Insignia.%s");
	this->Parachute_Anim.Read(exINI, section, "Parachute.Anim");

	// new on 08.11.09 for #342 (Operator=)
	if(pINI->ReadString(section, "Operator", "", Ares::readBuffer)) { // try to read the flag
		this->IsAPromiscuousWhoreAndLetsAnyoneRideIt = (strcmp(Ares::readBuffer, "_ANY_") == 0); // set whether this type accepts all operators
		if(!this->IsAPromiscuousWhoreAndLetsAnyoneRideIt) { // if not, find the specific operator it allows
			if(auto const pOperator = InfantryTypeClass::Find(Ares::readBuffer)) {
				this->Operator = pOperator;
			} else if(!INIClass::IsBlank(Ares::readBuffer)) {
				Debug::INIParseFailed(section, "Operator", Ares::readBuffer);
			}
		}
	}

	this->InitialPayload_Types.Read(exINI, section, "InitialPayload.Types");
	this->InitialPayload_Nums.Read(exINI, section, "InitialPayload.Nums");

	this->CameoPal.LoadFromINI(CCINIClass::INI_Art, pThis->ImageFile, "CameoPalette");

	if(pINI->ReadString(section, "Prerequisite.StolenTechs", "", Ares::readBuffer)) {
		this->RequiredStolenTech.reset();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			signed int idx = atoi(cur);
			if(idx > -1 && idx < 32) {
				this->RequiredStolenTech.set(idx);
			} else if(idx != -1) {
				Debug::INIParseFailed(section, "Prerequisite.StolenTechs", cur, "Expected a number between 0 and 31 inclusive");
			}
		}
	}

	this->ImmuneToEMP.Read(exINI, section, "ImmuneToEMP");
	this->EMP_Modifier.Read(exINI, section, "EMP.Modifier");
	this->EMP_Sparkles.Read(exINI, section, "EMP.Sparkles");

	if(pINI->ReadString(section, "EMP.Threshold", "inair", Ares::readBuffer)) {
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

	if(pINI->ReadString(section, "VeteranAbilities", "", Ares::readBuffer)) {
		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			if(!_strcmpi(cur, "empimmune")) {
				this->VeteranAbilityEMPIMMUNE = true;
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	if(pINI->ReadString(section, "EliteAbilities", "", Ares::readBuffer)) {
		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			if(!_strcmpi(cur, "empimmune")) {
				this->EliteAbilityEMPIMMUNE = true;
			}
		}
	}

	// #733
	this->ProtectedDriver.Read(exINI, section, "ProtectedDriver");
	this->ProtectedDriver_MinHealth.Read(exINI, section, "ProtectedDriver.MinHealth");
	this->CanDrive.Read(exINI, section, "CanDrive");

	// #346, #464, #970, #1014
	this->PassengersGainExperience.Read(exINI, section, "Experience.PromotePassengers");
	this->ExperienceFromPassengers.Read(exINI, section, "Experience.FromPassengers");
	this->PassengerExperienceModifier.Read(exINI, section, "Experience.PassengerModifier");
	this->MindControlExperienceSelfModifier.Read(exINI, section, "Experience.MindControlSelfModifier");
	this->MindControlExperienceVictimModifier.Read(exINI, section, "Experience.MindControlVictimModifier");
	this->SpawnExperienceOwnerModifier.Read(exINI, section, "Experience.SpawnOwnerModifier");
	this->SpawnExperienceSpawnModifier.Read(exINI, section, "Experience.SpawnModifier");
	this->ExperienceFromAirstrike.Read(exINI, section, "Experience.FromAirstrike");
	this->AirstrikeExperienceModifier.Read(exINI, section, "Experience.AirstrikeModifier");
	this->Insignia_ShowEnemy.Read(exINI, section, "Insignia.ShowEnemy");

	this->VoiceRepair.Read(exINI, section, "VoiceIFVRepair");

	this->VoiceAirstrikeAttack.Read(exINI, section, "VoiceAirstrikeAttack");
	this->VoiceAirstrikeAbort.Read(exINI, section, "VoiceAirstrikeAbort");

	this->HijackerEnterSound.Read(exINI, section, "VehicleThief.EnterSound");
	this->HijackerLeaveSound.Read(exINI, section, "VehicleThief.LeaveSound");
	this->HijackerKillPilots.Read(exINI, section, "VehicleThief.KillPilots");
	this->HijackerBreakMindControl.Read(exINI, section, "VehicleThief.BreakMindControl");
	this->HijackerAllowed.Read(exINI, section, "VehicleThief.Allowed");
	this->HijackerOneTime.Read(exINI, section, "VehicleThief.OneTime");

	this->IronCurtain_Modifier.Read(exINI, section, "IronCurtain.Modifier");

	this->ForceShield_Modifier.Read(exINI, section, "ForceShield.Modifier");

	this->Chronoshift_Allow.Read(exINI, section, "Chronoshift.Allow");
	this->Chronoshift_IsVehicle.Read(exINI, section, "Chronoshift.IsVehicle");

	this->CameoPCX.Read(CCINIClass::INI_Art, pThis->ImageFile, "CameoPCX");
	this->AltCameoPCX.Read(CCINIClass::INI_Art, pThis->ImageFile, "AltCameoPCX");

	this->CanBeReversed.Read(exINI, section, "CanBeReversed");

	// #305
	this->RadarJamRadius.Read(exINI, section, "RadarJamRadius");

	// #1208
	this->PassengerTurret.Read(exINI, section, "PassengerTurret");
	
	// #617 powered units
	this->PoweredBy.Read(exINI, section, "PoweredBy");

	//#1623 - AttachEffect on unit-creation
	this->AttachedTechnoEffect.Read(exINI);

	this->BuiltAt.Read(exINI, section, "BuiltAt");

	this->Cloneable.Read(exINI, section, "Cloneable");

	this->ClonedAt.Read(exINI, section, "ClonedAt");

	this->CarryallAllowed.Read(exINI, section, "Carryall.Allowed");
	this->CarryallSizeLimit.Read(exINI, section, "Carryall.SizeLimit");

	// #680, 1362
	this->ImmuneToAbduction.Read(exINI, section, "ImmuneToAbduction");

	this->FactoryOwners.Read(exINI, section, "FactoryOwners");
	this->ForbiddenFactoryOwners.Read(exINI, section, "FactoryOwners.Forbidden");
	this->FactoryOwners_HaveAllPlans.Read(exINI, section, "FactoryOwners.HaveAllPlans");

	// issue #896235: cyclic gattling
	this->GattlingCyclic.Read(exINI, section, "Gattling.Cycle");

	// #245 custom missiles
	if(auto pAircraftType = specific_cast<AircraftTypeClass*>(pThis)) {
		this->IsCustomMissile.Read(exINI, section, "Missile.Custom");
		this->CustomMissileData.Read(exINI, section, "Missile");
		this->CustomMissileData.GetEx()->Type = pAircraftType;
		this->CustomMissileWarhead.Read(exINI, section, "Missile.Warhead");
		this->CustomMissileEliteWarhead.Read(exINI, section, "Missile.EliteWarhead");
		this->CustomMissileTakeoffAnim.Read(exINI, section, "Missile.TakeOffAnim");
		this->CustomMissileTrailerAnim.Read(exINI, section, "Missile.TrailerAnim");
		this->CustomMissileTrailerSeparation.Read(exINI, section, "Missile.TrailerSeparation");
		this->CustomMissileWeapon.Read(exINI, section, "Missile.Weapon");
		this->CustomMissileEliteWeapon.Read(exINI, section, "Missile.EliteWeapon");
	}

	// non-crashable aircraft
	this->Crashable.Read(exINI, section, "Crashable");

	this->CrashSpin.Read(exINI, section, "CrashSpin");

	this->AirRate.Read(exINI, section, "AirRate");

	// tiberium
	this->TiberiumProof.Read(exINI, section, "TiberiumProof");
	this->TiberiumRemains.Read(exINI, section, "TiberiumRemains");
	this->TiberiumSpill.Read(exINI, section, "TiberiumSpill");
	this->TiberiumTransmogrify.Read(exINI, section, "TiberiumTransmogrify");

	// refinery and storage
	this->Refinery_UseStorage.Read(exINI, section, "Refinery.UseStorage");

	// cloak
	this->CloakSound.Read(exINI, section, "CloakSound");
	this->DecloakSound.Read(exINI, section, "DecloakSound");
	this->CloakPowered.Read(exINI, section, "Cloakable.Powered");
	this->CloakDeployed.Read(exINI, section, "Cloakable.Deployed");
	this->CloakAllowed.Read(exINI, section, "Cloakable.Allowed");
	this->CloakStages.Read(exINI, section, "Cloakable.Stages");

	// sensors
	this->SensorArray_Warn.Read(exINI, section, "SensorArray.Warn");

	this->EVA_UnitLost.Read(exINI, section, "EVA.Lost");

	// linking units for type selection
	this->GroupAs.Read(pINI, section, "GroupAs");

	// crew settings
	this->Crew_TechnicianChance.Read(exINI, section, "Crew.TechnicianChance");
	this->Crew_EngineerChance.Read(exINI, section, "Crew.EngineerChance");

	// drain settings
	this->Drain_Local.Read(exINI, section, "Drain.Local");
	this->Drain_Amount.Read(exINI, section, "Drain.Amount");

	// smoke when damaged
	this->SmokeAnim.Read(exINI, section, "Smoke.Anim");
	this->SmokeChanceRed.Read(exINI, section, "Smoke.ChanceRed");
	this->SmokeChanceDead.Read(exINI, section, "Smoke.ChanceDead");

	// hunter seeker
	this->HunterSeekerDetonateProximity.Read(exINI, section, "HunterSeeker.DetonateProximity");
	this->HunterSeekerDescendProximity.Read(exINI, section, "HunterSeeker.DescendProximity");
	this->HunterSeekerAscentSpeed.Read(exINI, section, "HunterSeeker.AscentSpeed");
	this->HunterSeekerDescentSpeed.Read(exINI, section, "HunterSeeker.DescentSpeed");
	this->HunterSeekerEmergeSpeed.Read(exINI, section, "HunterSeeker.EmergeSpeed");
	this->HunterSeekerIgnore.Read(exINI, section, "HunterSeeker.Ignore");

	this->CivilianEnemy.Read(exINI, section, "CivilianEnemy");

	// particles
	this->DamageSparks.Read(exINI, section, "DamageSparks");

	this->ParticleSystems_DamageSmoke.Read(exINI, section, "DamageSmokeParticleSystems");
	this->ParticleSystems_DamageSparks.Read(exINI, section, "DamageSparksParticleSystems");

	// berserking options
	this->BerserkROFMultiplier.Read(exINI, section, "Berserk.ROFMultiplier");

	// super weapon
	this->DesignatorRange.Read(exINI, section, "DesignatorRange");
	this->InhibitorRange.Read(exINI, section, "InhibitorRange");

	// assault options
	this->AssaulterLevel.Read(exINI, section, "Assaulter.Level");

	// crush
	this->OmniCrusher_Aggressive.Read(exINI, section, "OmniCrusher.Aggressive");
	this->CrushDamage.Read(exINI, section, "CrushDamage.%s");
	this->CrushDamageWarhead.Read(exINI, section, "CrushDamage.Warhead");

	//this->ReloadRate.Read(exINI, section, "ReloadRate");

	//this->ReloadAmount.Read(exINI, section, "ReloadAmount");
	//this->EmptyReloadAmount.Read(exINI, section, "EmptyReloadAmount");

	this->Saboteur.Read(exINI, section, "Saboteur");

	// note the wrong spelling of the tag for consistency
	this->CanPassiveAcquire_Guard.Read(exINI, section, "CanPassiveAquire.Guard");
	this->CanPassiveAcquire_Cloak.Read(exINI, section, "CanPassiveAquire.Cloak");

	// self healing
	//this->SelfHealing_Rate.Read(exINI, section, "SelfHealing.Rate");
	//this->SelfHealing_Amount.Read(exINI, section, "SelfHealing.%sAmount");
	//this->SelfHealing_Max.Read(exINI, section, "SelfHealing.%sMax");

	this->PassengersWhitelist.Read(exINI, section, "Passengers.Allowed");
	this->PassengersBlacklist.Read(exINI, section, "Passengers.Disallowed");

	this->NoManualUnload.Read(exINI, section, "NoManualUnload");
	this->NoManualFire.Read(exINI, section, "NoManualFire");
	//this->NoManualEnter.Read(exINI, section, "NoManualEnter");

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
		pData->Weapons.SetCapacity(WeaponCount, nullptr);
		pData->Weapons.set_Count(WeaponCount);
	}

	while(WeaponCount < pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.RemoveItem(pData->EliteWeapons.get_Count() - 1);
	}
	if(WeaponCount > pData->EliteWeapons.get_Count())
	{
		pData->EliteWeapons.SetCapacity(WeaponCount, nullptr);
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

	pINI->ReadString(section, prefix, "", buffer);

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

const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return this->GroupAs ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if(auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType))) {
		return pExt->GetSelectionGroupID();
	}

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);
	return (_strcmpi(id, pID) == 0);
}

bool TechnoTypeExt::ExtData::CameoIsElite(HouseClass const* const pHouse) const
{
	auto const pCountry = pHouse->Type;

	auto const pType = this->OwnerObject();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(!pType->AltCameo && !pExt->AltCameoPCX.Exists()) {
		return false;
	}

	switch(pType->WhatAmI()) {
	case AbstractType::InfantryType:
		if(pHouse->BarracksInfiltrated && !pType->Naval && pType->Trainable) {
			return true;
		} else {
			return pCountry->VeteranInfantry.FindItemIndex(static_cast<InfantryTypeClass*>(pType)) != -1;
		}
	case AbstractType::UnitType:
		if(pHouse->WarFactoryInfiltrated && !pType->Naval && pType->Trainable) {
			return true;
		} else {
			return pCountry->VeteranUnits.FindItemIndex(static_cast<UnitTypeClass*>(pType)) != -1;
		}
	case AbstractType::AircraftType:
		return pCountry->VeteranAircraft.FindItemIndex(static_cast<AircraftTypeClass*>(pType)) != -1;
	case AbstractType::BuildingType:
		if(auto const pItem = pType->UndeploysInto) {
			return pCountry->VeteranUnits.FindItemIndex(static_cast<UnitTypeClass*>(pItem)) != -1;
		} else {
			auto const pData = HouseTypeExt::ExtMap.Find(pCountry);
			return pData->VeteranBuildings.Contains(static_cast<BuildingTypeClass*>(pType));
		}
	}

	return false;
}

bool TechnoTypeExt::ExtData::CanBeBuiltAt(
	BuildingTypeClass const* const pFactoryType) const
{
	auto const pBExt = BuildingTypeExt::ExtMap.Find(pFactoryType);
	return (this->BuiltAt.empty() && !pBExt->Factory_ExplicitOnly)
		|| this->BuiltAt.Contains(pFactoryType);
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

bool TechnoTypeExt::ExtData::IsGenericPrerequisite() const
{
	if(this->GenericPrerequisite.empty()) {
		bool isGeneric = false;
		for(auto const& Prereq : GenericPrerequisite::Array) {
			if(Prereq->Alternates.FindItemIndex(this->OwnerObject()) != -1) {
				isGeneric = true;
				break;
			}
		}
		this->GenericPrerequisite = isGeneric;
	}

	return this->GenericPrerequisite;
}

// =============================
// load / save

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Survivors_Pilots)
		.Process(this->Survivors_PilotChance)
		.Process(this->Survivors_PassengerChance)
		.Process(this->Survivors_PilotCount)
		.Process(this->Crew_TechnicianChance)
		.Process(this->Crew_EngineerChance)
		.Process(this->PrerequisiteLists)
		.Process(this->PrerequisiteNegatives)
		.Process(this->PrerequisiteTheaters)
		.Process(this->GenericPrerequisite)
		.Process(this->Secret_RequiredHouses)
		.Process(this->Secret_ForbiddenHouses)
		.Process(this->Is_Deso)
		.Process(this->Is_Deso_Radiation)
		.Process(this->Is_Cow)
		.Process(this->Is_Spotlighted)
		.Process(this->Spot_Height)
		.Process(this->Spot_Distance)
		.Process(this->Spot_AttachedTo)
		.Process(this->Spot_DisableR)
		.Process(this->Spot_DisableG)
		.Process(this->Spot_DisableB)
		.Process(this->Spot_Reverse)
		.Process(this->Is_Bomb)
		//.Process(this->Weapons)
		//.Process(this->EliteWeapons)
		.Process(this->Insignia)
		.Process(this->Insignia_ShowEnemy)
		.Process(this->Parachute_Anim)
		.Process(this->Operator)
		.Process(this->IsAPromiscuousWhoreAndLetsAnyoneRideIt)
		.Process(this->CameoPal)
		.Process(this->InitialPayload_Types)
		.Process(this->InitialPayload_Nums)
		.Process(this->RequiredStolenTech)
		.Process(this->ImmuneToEMP)
		.Process(this->VeteranAbilityEMPIMMUNE)
		.Process(this->EliteAbilityEMPIMMUNE)
		.Process(this->EMP_Threshold)
		.Process(this->EMP_Modifier)
		.Process(this->EMP_Sparkles)
		.Process(this->IronCurtain_Modifier)
		.Process(this->ForceShield_Modifier)
		.Process(this->Chronoshift_Allow)
		.Process(this->Chronoshift_IsVehicle)
		.Process(this->ProtectedDriver)
		.Process(this->ProtectedDriver_MinHealth)
		.Process(this->CanDrive)
		.Process(this->AlternateTheaterArt)
		.Process(this->PassengersGainExperience)
		.Process(this->ExperienceFromPassengers)
		.Process(this->PassengerExperienceModifier)
		.Process(this->MindControlExperienceSelfModifier)
		.Process(this->MindControlExperienceVictimModifier)
		.Process(this->SpawnExperienceOwnerModifier)
		.Process(this->SpawnExperienceSpawnModifier)
		.Process(this->ExperienceFromAirstrike)
		.Process(this->AirstrikeExperienceModifier)
		.Process(this->VoiceRepair)
		.Process(this->VoiceAirstrikeAttack)
		.Process(this->VoiceAirstrikeAbort)
		.Process(this->HijackerEnterSound)
		.Process(this->HijackerLeaveSound)
		.Process(this->HijackerKillPilots)
		.Process(this->HijackerBreakMindControl)
		.Process(this->HijackerAllowed)
		.Process(this->HijackerOneTime)
		.Process(this->WaterImage)
		.Process(this->CloakSound)
		.Process(this->DecloakSound)
		.Process(this->CloakPowered)
		.Process(this->CloakDeployed)
		.Process(this->CloakAllowed)
		.Process(this->CloakStages)
		.Process(this->SensorArray_Warn)
		.Process(this->CameoPCX)
		.Process(this->AltCameoPCX)
		.Process(this->GroupAs)
		.Process(this->ReversedByHouses)
		.Process(this->CanBeReversed)
		.Process(this->RadarJamRadius)
		.Process(this->PassengerTurret)
		.Process(this->PoweredBy)
		.Process(this->AttachedTechnoEffect)
		.Process(this->BuiltAt)
		.Process(this->Cloneable)
		.Process(this->ClonedAt)
		.Process(this->CarryallAllowed)
		.Process(this->CarryallSizeLimit)
		.Process(this->ImmuneToAbduction)
		.Process(this->FactoryOwners)
		.Process(this->ForbiddenFactoryOwners)
		.Process(this->FactoryOwners_HaveAllPlans)
		.Process(this->GattlingCyclic)
		.Process(this->Crashable)
		.Process(this->CrashSpin)
		.Process(this->AirRate)
		.Process(this->CivilianEnemy)
		.Process(this->IsCustomMissile)
		.Process(this->CustomMissileData)
		.Process(this->CustomMissileWarhead)
		.Process(this->CustomMissileEliteWarhead)
		.Process(this->CustomMissileTakeoffAnim)
		.Process(this->CustomMissileTrailerAnim)
		.Process(this->CustomMissileTrailerSeparation)
		.Process(this->CustomMissileWeapon)
		.Process(this->CustomMissileEliteWeapon)
		.Process(this->TiberiumProof)
		.Process(this->TiberiumRemains)
		.Process(this->TiberiumSpill)
		.Process(this->TiberiumTransmogrify)
		.Process(this->Refinery_UseStorage)
		.Process(this->EVA_UnitLost)
		.Process(this->Drain_Local)
		.Process(this->Drain_Amount)
		.Process(this->SmokeChanceRed)
		.Process(this->SmokeChanceDead)
		.Process(this->SmokeAnim)
		.Process(this->HunterSeekerDetonateProximity)
		.Process(this->HunterSeekerDescendProximity)
		.Process(this->HunterSeekerAscentSpeed)
		.Process(this->HunterSeekerDescentSpeed)
		.Process(this->HunterSeekerEmergeSpeed)
		.Process(this->HunterSeekerIgnore)
		.Process(this->DesignatorRange)
		.Process(this->InhibitorRange)
		.Process(this->DamageSparks)
		.Process(this->ParticleSystems_DamageSmoke)
		.Process(this->ParticleSystems_DamageSparks)
		.Process(this->BerserkROFMultiplier)
		.Process(this->AssaulterLevel)
		.Process(this->OmniCrusher_Aggressive)
		.Process(this->CrushDamage)
		.Process(this->CrushDamageWarhead)
		.Process(this->ReloadRate)
		.Process(this->ReloadAmount)
		.Process(this->EmptyReloadAmount)
		.Process(this->Saboteur)
		.Process(this->CanPassiveAcquire_Guard)
		.Process(this->CanPassiveAcquire_Cloak)
		.Process(this->SelfHealing_Rate)
		.Process(this->SelfHealing_Amount)
		.Process(this->SelfHealing_Max)
		.Process(this->PassengersWhitelist)
		.Process(this->PassengersBlacklist)
		.Process(this->NoManualUnload)
		.Process(this->NoManualFire)
		.Process(this->NoManualEnter);
}

void TechnoTypeExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<TechnoTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") {
}

TechnoTypeExt::ExtContainer::~ExtContainer() = default;

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
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

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

DEFINE_HOOK(679CAF, RulesClass_LoadAfterTypeData_CompleteInitialization, 5) {
	//GET(CCINIClass*, pINI, ESI);

	for(auto const& pType : *BuildingTypeClass::Array) {
		auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
		pExt->CompleteInitialization();
	}

	return 0;
}

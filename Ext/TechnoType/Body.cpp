#include "Body.h"
#include "../Side/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"

#include <AnimTypeClass.h>
#include <Theater.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x44444444;
Container<TechnoTypeExt> TechnoTypeExt::ExtMap;

template<> TechnoTypeExt::TT *Container<TechnoTypeExt>::SavingObject = NULL;
template<> IStream *Container<TechnoTypeExt>::SavingStream = NULL;

// =============================
// member funcs

void TechnoTypeExt::ExtData::Initialize(TechnoTypeClass *pThis) {
	this->Survivors_PilotChance.SetAll(int(RulesClass::Instance->CrewEscape * 100));
	this->Survivors_PassengerChance.SetAll(-1); // was (int)RulesClass::Global()->CrewEscape * 100); - changed to -1 to indicate "100% if this is a land transport"

	this->Survivors_Pilots.SetCapacity(SideClass::Array->Count, NULL);

	this->Survivors_PilotCount = pThis->Crewed; // should be 0 if false, 1 if true

	for(int i = 0; i < SideClass::Array->Count; ++i) {
		this->Survivors_Pilots[i] = SideExt::ExtMap.Find(SideClass::Array->Items[i])->Crew;
	}

	this->PrerequisiteLists.SetCapacity(0, NULL);
	this->PrerequisiteLists.AddItem(new DynamicVectorClass<int>);

	this->PrerequisiteTheaters = 0xFFFFFFFF;

	this->Secret_RequiredHouses = 0xFFFFFFFF;
	this->Secret_ForbiddenHouses = 0;

	this->Is_Deso = this->Is_Deso_Radiation = !strcmp(pThis->ID, "DESO");
	this->Is_Cow = !strcmp(pThis->ID, "COW");
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

	this->Survivors_PilotCount = pINI->ReadInteger(section, "Survivor.Pilots", this->Survivors_PilotCount);

	this->Survivors_PilotChance.LoadFromINI(pINI, section, "Survivor.%sPilotChance");
	this->Survivors_PassengerChance.LoadFromINI(pINI, section, "Survivor.%sPassengerChance");

	char flag[256];
	for(int i = 0; i < SideClass::Array->Count; ++i) {
		_snprintf(flag, 256, "Survivor.Side%d", i);
		if(pINI->ReadString(section, flag, "", Ares::readBuffer, Ares::readLength)) {
			this->Survivors_Pilots[i] = InfantryTypeClass::Find(Ares::readBuffer);
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

	DynamicVectorClass<int> *dvc = this->PrerequisiteLists.GetItem(0);
	if(pINI->ReadString(section, "Prerequisite", "", Ares::readBuffer, Ares::readLength)) {
		Prereqs::Parse(Ares::readBuffer, dvc);
	}
	if(pINI->ReadString(section, "PrerequisiteOverride", "", Ares::readBuffer, Ares::readLength)) {
		dvc = pThis->get_PrerequisiteOverride();
		Prereqs::Parse(Ares::readBuffer, dvc);
	}
	for(int i = 0; i < this->PrerequisiteLists.Count; ++i) {
		_snprintf(flag, 256, "Prerequisite.List%d", i);
		if(pINI->ReadString(section, flag, "", Ares::readBuffer, Ares::readLength)) {
			dvc = this->PrerequisiteLists.GetItem(i);
			Prereqs::Parse(Ares::readBuffer, dvc);
		}
	}

	dvc = &this->PrerequisiteNegatives;
	if(pINI->ReadString(section, "Prerequisite.Negative", "", Ares::readBuffer, Ares::readLength)) {
		Prereqs::Parse(Ares::readBuffer, dvc);
	}

	if(pINI->ReadString(section, "Prerequisite.RequiredTheaters", "", Ares::readBuffer, Ares::readLength)) {
		this->PrerequisiteTheaters = 0;
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			signed int idx = Theater::FindIndex(cur);
			if(idx != -1) {
				this->PrerequisiteTheaters |= (1 << idx);
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
	this->Insignia.LoadFromINI(pINI, section, "Insignia.%s");
	this->Parachute_Anim.Parse(&exINI, section, "Parachute.Anim");

	// new on 08.11.09 for #342 (Operator=)
	if(pINI->ReadString(section, "Operator", "", Ares::readBuffer, Ares::readLength)) { // try to read the flag
		this->IsAPromiscuousWhoreAndLetsAnyoneRideIt = (strcmp(Ares::readBuffer, "_ANY_") == 0); // set whether this type accepts all operators
		if(!this->IsAPromiscuousWhoreAndLetsAnyoneRideIt) { // if not, find the specific operator it allows
			this->Operator = InfantryTypeClass::Find(Ares::readBuffer);
		}
	}

	this->CameoPal.LoadFromINI(CCINIClass::INI_Art, pThis->ImageFile, "CameoPalette");

	if(pINI->ReadString(section, "Prerequisite.StolenTechs", "", Ares::readBuffer, Ares::readLength)) {
		this->RequiredStolenTech.reset();
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			signed int idx = atoi(cur);
			if(idx > -1 && idx < 32) {
				this->RequiredStolenTech.set(idx);
			}
		}
	}

	this->ImmuneToEMP.Read(&exINI, section, "ImmuneToEMP");

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

// =============================
// load/save

void Container<TechnoTypeExt>::Save(TechnoTypeClass *pThis, IStream *pStm) {
	TechnoTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;
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

	ULONG out;

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

DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
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

DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

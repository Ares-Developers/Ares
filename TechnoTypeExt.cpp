#include "TechnoTypeExt.h"
#include "SideExt.h"
#include "Prerequisites.h"

EXT_P_DEFINE(TechnoTypeClass);

EXT_CTOR(TechnoTypeClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Survivors_Pilots.SetCapacity(0, NULL);

		pData->Survivors_PilotChance = 0;
		pData->Survivors_PassengerChance = 0;

		pData->Is_Deso = pData->Is_Deso_Radiation = 0;
		pData->Is_Cow = 0;

		pData->Weapons.SetCapacity(0, NULL);
		pData->EliteWeapons.SetCapacity(0, NULL);

		pData->Insignia_R = pData->Insignia_V = pData->Insignia_E = NULL;

		pData->Data_Initialized = 0;

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

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

void TechnoTypeClassExt::TechnoTypeClassData::Initialize(TechnoTypeClass *pThis)
{
	this->Survivors_Pilots.SetCapacity(SideClass::Array->get_Count(), NULL);

	this->Survivors_PilotChance = (int)(RulesClass::Global()->get_CrewEscape() * 100);
	this->Survivors_PassengerChance = (int)(RulesClass::Global()->get_CrewEscape() * 100);

	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
		this->Survivors_Pilots[i] = SideExt::Map[SideClass::Array->GetItem(i)].Crew;

	this->PrerequisiteLists.SetCapacity(0, NULL);
	this->PrerequisiteLists.AddItem(new DynamicVectorClass<int>);

	this->PrerequisiteTheaters = 0xFFFFFFFF;

	this->Secret_RequiredHouses = 0xFFFFFFFF;
	this->Secret_ForbiddenHouses = 0;

	this->Is_Deso = this->Is_Deso_Radiation = !strcmp(pThis->get_ID(), "DESO");

	this->Data_Initialized = 1;
}

EXT_LOAD_INI(TechnoTypeClass)
{
	const char * section = pThis->get_ID();
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];
	if(!pData->Data_Initialized)
	{
		pData->Initialize(pThis);
	}

	// survivors
	pData->Survivors_Pilots.SetCapacity(SideClass::Array->get_Count(), NULL);

	pData->Survivors_PilotChance = pINI->ReadInteger(section, "Survivor.PilotChance", pData->Survivors_PilotChance);
	pData->Survivors_PassengerChance = pINI->ReadInteger(section, "Survivor.PassengerChance", pData->Survivors_PassengerChance);

	char buffer[BUFLEN];
	char flag[256];
	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
	{
		sprintf(flag, "Survivor.Side%d", i);
		PARSE_INFANTRY(flag, pData->Survivors_Pilots[i]);
	}

	// prereqs
	int PrereqListLen = pINI->ReadInteger(section, "Prerequisite.Lists", pData->PrerequisiteLists.get_Count() - 1);

	if(PrereqListLen < 1)
	{
		PrereqListLen = 0;
	}
	++PrereqListLen;
	while(PrereqListLen > pData->PrerequisiteLists.get_Count())
	{
		pData->PrerequisiteLists.AddItem(new DynamicVectorClass<int>);
	}

	DynamicVectorClass<int> *dvc = pData->PrerequisiteLists.GetItem(0);
	if(pINI->ReadString(section, "Prerequisite", "", buffer, BUFLEN))
	{
		Prereqs::Parse(buffer, dvc);
	}
	for(int i = 0; i < pData->PrerequisiteLists.get_Count(); ++i)
	{
		sprintf(flag, "Prerequisite.List%d", i);
		if(pINI->ReadString(section, flag, "", buffer, BUFLEN))
		{
			dvc = pData->PrerequisiteLists.GetItem(i);
			Prereqs::Parse(buffer, dvc);
		}
	}

	dvc = &pData->PrerequisiteNegatives;
	if(pINI->ReadString(section, "Prerequisite.Negative", "", buffer, BUFLEN))
	{
		Prereqs::Parse(buffer, dvc);
	}

	if(pINI->ReadString(section, "Prerequisite.RequiredTheaters", "", buffer, BUFLEN))
	{
//		DEBUGLOG("[%s]Theater text: %s\n", section, buffer);
		pData->PrerequisiteTheaters = 0;
		for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ","))
		{
			signed int idx = Theater::FindIndex(cur);
//			DEBUGLOG("[%s]Theater token %s maps to %d\n", section, cur, idx);
			if(idx > -1)
			{
				pData->PrerequisiteTheaters |= (1 << idx);
//				DEBUGLOG("[%s]Theater token bitfield: %X\n", section, pData->PrerequisiteTheaters);
			}
		}
//		DEBUGLOG("[%s]Theaters: %X\n", section, pData->PrerequisiteTheaters);
	}


	// new secret lab
	pData->Secret_RequiredHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.RequiredHouses", pData->Secret_RequiredHouses);

	pData->Secret_ForbiddenHouses
		= pINI->ReadHouseTypesList(section, "SecretLab.ForbiddenHouses", pData->Secret_ForbiddenHouses);

	pData->Is_Deso = pINI->ReadBool(section, "IsDesolator", pData->Is_Deso);
	pData->Is_Deso_Radiation = pINI->ReadBool(section, "IsDesolator.RadDependant", pData->Is_Deso_Radiation);
	pData->Is_Cow  = pINI->ReadBool(section, "IsCow", pData->Is_Cow);

	// insignia
	SHPStruct *image = NULL;
	if(pINI->ReadString(section, "Insignia.Rookie", "", buffer, 256)) {
		sprintf(flag, "%s.shp", buffer);
		image = FileSystem::LoadSHPFile(flag);
		if(image) {
			pData->Insignia_R = image;
		}
	}
	if(pINI->ReadString(section, "Insignia.Veteran", "", buffer, 256)) {
		sprintf(flag, "%s.shp", buffer);
		image = FileSystem::LoadSHPFile(flag);
		if(image) {
			pData->Insignia_V = image;
		}
	}
	if(pINI->ReadString(section, "Insignia.Elite", "", buffer, 256)) {
		sprintf(flag, "%s.shp", buffer);
		image = FileSystem::LoadSHPFile(flag);
		if(image) {
			pData->Insignia_E = image;
		}
	}

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
		sprintf(flag, "Weapon%d", i);
		ReadWeapon(W, flag, section, pINI);

		W = &pData->EliteWeapons[i];
		sprintf(flag, "EliteWeapon%d", i);
		ReadWeapon(W, flag, section, pINI);
	}
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
		sprintf(flag, "%sFLH", prefix);
	}
	else
	{
		sprintf(flag, "%sFireFLH", prefix);
	}
	pArtINI->Read3Integers((int *)&FLH, section, flag, (int *)&pWeapon->FLH);
	pWeapon->FLH = FLH;

	sprintf(flag, "%sBarrelLength", prefix);
	pWeapon->BarrelLength = pArtINI->ReadInteger(section, flag, pWeapon->BarrelLength);
	sprintf(flag, "%sBarrelThickness", prefix);
	pWeapon->BarrelThickness = pArtINI->ReadInteger(section, flag, pWeapon->BarrelThickness);
	sprintf(flag, "%sTurretLocked", prefix);
	pWeapon->TurretLocked = pArtINI->ReadBool(section, flag, pWeapon->TurretLocked);
}


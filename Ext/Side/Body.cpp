#include "Body.h"

//Static init
const DWORD Extension<SideClass>::Canary = 0x87654321;
Container<SideExt> SideExt::ExtMap;

SideExt::TT *Container<SideExt>::SavingObject = NULL;
IStream *Container<SideExt>::SavingStream = NULL;

stdext::hash_map<VoxClass*, DynamicVectorClass<SideExt::VoxFileNameStruct> > SideExt::EVAFiles;

void SideExt::ExtData::Initialize(SideClass *pThis)
{
	char* pID = pThis->get_ID();

	//are these necessary?
	this->BaseDefenseCounts.Clear();
	this->BaseDefenses.Clear();
	this->ParaDrop.Clear();
	this->ParaDropNum.Clear();

	if(!_strcmpi(pID, "Nod")) { //Soviets

		for(int i = 0; i < RulesClass::Global()->get_SovietBaseDefenseCounts()->Count; ++i) {
			this->BaseDefenseCounts.AddItem(RulesClass::Global()->get_SovietBaseDefenseCounts()->GetItem(i));
		}

		for(int i = 0; i < RulesClass::Global()->get_SovietBaseDefenses()->Count; ++i) {
			this->BaseDefenses.AddItem(RulesClass::Global()->get_SovietBaseDefenses()->GetItem(i));
		}

		this->Crew = RulesClass::Global()->SovietCrew;
		this->DefaultDisguise = RulesClass::Global()->SovietDisguise;
		strcpy(this->EVATag, "Russian");
		this->LoadTextColor = ColorScheme::Find("SovietLoad");
		
		for(int i = 0; i < RulesClass::Global()->get_SovParaDropInf()->Count; ++i) {
			this->ParaDrop.AddItem((RulesClass::Global()->get_SovParaDropInf()->GetItem(i)));
		}

		for(int i = 0; i < RulesClass::Global()->get_SovParaDropNum()->Count; ++i) {
			this->ParaDropNum.AddItem(RulesClass::Global()->get_SovParaDropNum()->GetItem(i));
		}

//						this->PowerPlant = RulesClass::Global()->get_NodRegularPower();
		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = false;
		this->SurvivorDivisor = RulesClass::Global()->SovietSurvivorDivisor;

	} else if(!_strcmpi(pID, "ThirdSide")) { //Yuri

		for(int i = 0; i < RulesClass::Global()->get_ThirdBaseDefenseCounts()->Count; ++i) {
			this->BaseDefenseCounts.AddItem(RulesClass::Global()->get_ThirdBaseDefenseCounts()->GetItem(i));
		}

		for(int i = 0; i < RulesClass::Global()->get_ThirdBaseDefenses()->Count; ++i) {
			this->BaseDefenses.AddItem(RulesClass::Global()->get_ThirdBaseDefenses()->GetItem(i));
		}

		this->Crew = RulesClass::Global()->ThirdCrew;
		this->DefaultDisguise = RulesClass::Global()->ThirdDisguise;
		strcpy(this->EVATag, "Yuri");
		this->LoadTextColor = ColorScheme::Find("SovietLoad");
		
		for(int i = 0; i < RulesClass::Global()->get_YuriParaDropInf()->Count; ++i) {
			this->ParaDrop.AddItem(RulesClass::Global()->get_YuriParaDropInf()->GetItem(i));
		}

		for(int i = 0; i < RulesClass::Global()->get_YuriParaDropNum()->Count; ++i) {
			this->ParaDropNum.AddItem(RulesClass::Global()->get_YuriParaDropNum()->GetItem(i));
		}

//						this->PowerPlant = RulesClass::Global()->get_ThirdPowerPlant();
		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = true;
		this->SurvivorDivisor = RulesClass::Global()->ThirdSurvivorDivisor;

	} else { //Allies or any other country

		for(int i = 0; i < RulesClass::Global()->get_AlliedBaseDefenseCounts()->Count; ++i) {
			this->BaseDefenseCounts.AddItem(RulesClass::Global()->get_AlliedBaseDefenseCounts()->GetItem(i));
		}

		for(int i = 0; i < RulesClass::Global()->get_AlliedBaseDefenses()->Count; ++i) {
			this->BaseDefenses.AddItem(RulesClass::Global()->get_AlliedBaseDefenses()->GetItem(i));
		}

		this->Crew = RulesClass::Global()->AlliedCrew;
		this->DefaultDisguise = RulesClass::Global()->AlliedDisguise;
		strcpy(this->EVATag, "Allied");
		this->LoadTextColor = ColorScheme::Find("AlliedLoad");

		for(int i = 0; i < RulesClass::Global()->get_AllyParaDropInf()->Count; ++i) {
			this->ParaDrop.AddItem(RulesClass::Global()->get_AllyParaDropInf()->GetItem(i));
		}

		for(int i = 0; i < RulesClass::Global()->get_AllyParaDropNum()->Count; ++i) {
			this->ParaDropNum.AddItem(RulesClass::Global()->get_AllyParaDropNum()->GetItem(i));
		}

//						this->PowerPlant = RulesClass::Global()->get_GDIPowerPlant();
		this->SidebarMixFileIndex = 1;
		this->SidebarYuriFileNames = false;
		this->SurvivorDivisor = RulesClass::Global()->AlliedSurvivorDivisor;
	}
	
	this->_Initialized = is_Inited;
};

void SideExt::ExtData::LoadFromINI(SideClass *pThis, CCINIClass *pINI)
{
	if(!pINI) {
		return;
	}

	char* pID = pThis->get_ID();

/*
	if(!_strcmpi(pID, "Civilian") || !_strcmpi(pID, "Mutant")) {
		continue;
	}
*/

	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	char* p = NULL;

	if(pINI->ReadString(pID, "AI.BaseDefenseCounts", "", Ares::readBuffer, Ares::readLength)) {
		this->BaseDefenseCounts.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			this->BaseDefenseCounts.AddItem(atoi(p));
		}
	}

	if(pINI->ReadString(pID, "AI.BaseDefenses", "", Ares::readBuffer, Ares::readLength)) {
		this->BaseDefenses.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			this->BaseDefenses.AddItem(BuildingTypeClass::FindOrAllocate(p));
		}
	}

	if(pINI->ReadString(pID, "Crew", "", Ares::readBuffer, 0x80)) {
		this->Crew = InfantryTypeClass::FindOrAllocate(Ares::readBuffer);
	}

	if(pINI->ReadString(pID, "DefaultDisguise", "", Ares::readBuffer, 0x80)) {
		this->DefaultDisguise = InfantryTypeClass::FindOrAllocate(Ares::readBuffer);
	}

	if(pINI->ReadString(pID, "EVA.Tag", "", Ares::readBuffer, 0x20)) {
		strncpy(this->EVATag, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "LoadScreenText.Color", "", Ares::readBuffer, 0x80)) {
		ColorScheme* CS = ColorScheme::Find(Ares::readBuffer);
		if(CS) {
			this->LoadTextColor = CS;
		}
	}

	if(pINI->ReadString(pID, "ParaDrop.Types", "", Ares::readBuffer, Ares::readLength)) {
		this->ParaDrop.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			TechnoTypeClass* pTT = UnitTypeClass::Find(p);
			
			if(!pTT) {
				pTT = InfantryTypeClass::Find(p);
			}

			if(pTT) {
				this->ParaDrop.AddItem(pTT);
			}
		}
	}

	if(pINI->ReadString(pID, "ParaDrop.Num", "", Ares::readBuffer, Ares::readLength)) {
		this->ParaDropNum.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			this->ParaDropNum.AddItem(atoi(p));
		}
	}

//					if(pINI->ReadString(pID, "AI.PowerPlant", "", buffer, 0x80))
//					pData->PowerPlant = BuildingTypeClass::FindOrAllocate(buffer);

	this->SidebarMixFileIndex =  pINI->ReadInteger(pID, "Sidebar.MixFileIndex", this->SidebarMixFileIndex);
	this->SidebarYuriFileNames = pINI->ReadBool(pID, "Sidebar.YuriFileNames", this->SidebarYuriFileNames);
	this->SurvivorDivisor =      pINI->ReadInteger(pID, "SurvivorDivisor", this->SurvivorDivisor);

}

DWORD SideExt::BaseDefenses(REGISTERS* R, DWORD dwReturnAddress)
{
	HouseTypeClass* pCountry = (HouseTypeClass*)R->get_EAX();

	int n = pCountry->SideIndex;
	SideClass* pSide = SideClass::Array->GetItem(n);
	SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide);
	if(pData) {
		R->set_EBX((DWORD)&pData->BaseDefenses);
		return dwReturnAddress;
	} else {
		return 0;
	}
}

DWORD SideExt::Disguise(REGISTERS* R, DWORD dwReturnAddress, bool bUseESI)
{
	HouseClass* pHouse = (HouseClass*)R->get_EAX();
	InfantryClass* pThis;
	
	if(bUseESI)
		pThis = (InfantryClass*)R->get_ESI();
	else
		pThis = (InfantryClass*)R->get_ECX();

	int n = pHouse->SideIndex;
	SideClass* pSide = SideClass::Array->GetItem(n);
	SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide);
	if(pData) {
		pThis->set_Disguise(pData->DefaultDisguise);
		return dwReturnAddress;
	} else {
		return 0;
	}
}

DWORD SideExt::LoadTextColor(REGISTERS* R, DWORD dwReturnAddress)
{
	int n = R->get_EAX();
	SideClass* pSide = SideClass::Array->GetItem(n);
	SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide);
	if(pData && pData->LoadTextColor) {
		R->set_EAX((DWORD)pData->LoadTextColor);
		return dwReturnAddress;
	} else {
		return 0;
	}
}

DWORD SideExt::MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	BYTE* pScenario = (BYTE*)R->get_EAX();	//Scenario, upate this once mapped!
	int n = *((int*)(pScenario + 0x34B8));

	SideClass* pSide = SideClass::Array->GetItem(n);
	SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide);
	if(pData) {
		if(pData->SidebarYuriFileNames) {
			return dwReturnAddress1;
		} else {
			return dwReturnAddress2;
		}
	} else {
		return 0;
	}
}

// =============================
// load/save

void Container<SideExt>::Save(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;
		pData->BaseDefenses.Save(pStm);
		pData->BaseDefenseCounts.Save(pStm);
		pData->ParaDrop.Save(pStm);
		pData->ParaDropNum.Save(pStm);
	}
}

void Container<SideExt>::Load(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->DefaultDisguise);
	SWIZZLE(pData->Crew);
	pData->BaseDefenses.Load(pStm, 1);
	pData->BaseDefenseCounts.Load(pStm, 0);
	pData->ParaDrop.Load(pStm, 1);
	pData->ParaDropNum.Load(pStm, 0);
}

// =============================
// container hooks

DEFINE_HOOK(6A4609, SideClass_CTOR, 7)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}


DEFINE_HOOK(6A4930, SideClass_DTOR, 6)
{
	GET(SideClass*, pItem, ECX);

	SideExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(6A4780, SideClass_SaveLoad_Prefix, 6)
DEFINE_HOOK_AGAIN(6A48A0, SideClass_SaveLoad_Prefix, 5)
{
	GET_STACK(SideExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8); 

	Container<SideExt>::SavingObject = pItem;
	Container<SideExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(6A488B, SideClass_Load_Suffix, 6)
{
	SideExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6A48FC, SideClass_Save_Suffix, 5)
{
	SideExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(679A10, SideClass_LoadAllFromINI, 5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);
	SideExt::ExtMap.LoadAllFromINI(pINI); // bwahaha

	return 0;
}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/
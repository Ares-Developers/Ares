#include "Body.h"
#include "../../Ares.CRT.h"
#include <ScenarioClass.h>

//Static init
template<> const DWORD Extension<SideClass>::Canary = 0x87654321;
Container<SideExt> SideExt::ExtMap;
int SideExt::CurrentLoadTextColor = -1;

template<> SideExt::TT *Container<SideExt>::SavingObject = nullptr;
template<> IStream *Container<SideExt>::SavingStream = nullptr;

void SideExt::ExtData::Initialize(SideClass *pThis)
{
	char* pID = pThis->ID;

	this->ArrayIndex = SideClass::FindIndex(pThis->ID);

	//are these necessary?
	this->ParaDrop.Clear();
	this->ParaDropNum.Clear();

	this->ParaDropPlane = AircraftTypeClass::FindIndex("PDPLANE");

	if(!_strcmpi(pID, "Nod")) { //Soviets

		this->EVAIndex = 1;

		this->ParaDropFallbackTypes = &RulesClass::Instance->SovParaDropInf;
		this->ParaDropFallbackNum = &RulesClass::Instance->SovParaDropNum;

		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = false;

		this->ToolTipTextColor = ColorStruct(255, 255, 0);
		this->MessageTextColorIndex = 11;

	} else if(!_strcmpi(pID, "ThirdSide")) { //Yuri

		this->EVAIndex = 2;

		this->ParaDropFallbackTypes = &RulesClass::Instance->YuriParaDropInf;
		this->ParaDropFallbackNum = &RulesClass::Instance->YuriParaDropNum;

		this->SidebarMixFileIndex = 2;
		this->SidebarYuriFileNames = true;

		this->ToolTipTextColor = ColorStruct(255, 255, 0);
		this->MessageTextColorIndex = 25;

	} else { //Allies or any other country

		this->EVAIndex = 0;

		this->ParaDropFallbackTypes = &RulesClass::Instance->AllyParaDropInf;
		this->ParaDropFallbackNum = &RulesClass::Instance->AllyParaDropNum;

		this->SidebarMixFileIndex = 1;
		this->SidebarYuriFileNames = false;

		this->ToolTipTextColor = ColorStruct(164, 210, 255);
		this->MessageTextColorIndex = 21;
	}

};

void SideExt::ExtData::LoadFromINIFile(SideClass *pThis, CCINIClass *pINI)
{
	char* p = nullptr;
	char* section = pThis->get_ID();

	INI_EX exINI(pINI);

	this->BaseDefenseCounts.Read(&exINI, section, "AI.BaseDefenseCounts");

	this->BaseDefenses.Read(&exINI, section, "AI.BaseDefenses");

	this->Crew.Parse(&exINI, section, "Crew");

	this->Engineer.Parse(&exINI, section, "Engineer");

	this->Technician.Parse(&exINI, section, "Technician");

	this->Disguise.Parse(&exINI, section, "DefaultDisguise");

	this->EVAIndex.Read(&exINI, section, "EVA.Tag");

	this->Parachute_Anim.Parse(&exINI, section, "Parachute.Anim");

	this->ParaDropPlane.Read(&exINI, section, "ParaDrop.Aircraft");

	if(pINI->ReadString(section, "ParaDrop.Types", "", Ares::readBuffer, Ares::readLength)) {
		this->ParaDrop.Clear();
		this->ParaDropFallbackTypes = nullptr;

		char* context = nullptr;
		for(p = strtok_s(Ares::readBuffer, Ares::readDelims, &context); p && *p; p = strtok_s(nullptr, Ares::readDelims, &context)) {
			TechnoTypeClass* pTT = UnitTypeClass::Find(p);

			if(!pTT) {
				pTT = InfantryTypeClass::Find(p);
			}

			if(pTT) {
				this->ParaDrop.AddItem(pTT);
			} else {
				Debug::INIParseFailed(section, "ParaDrop.Types", p);
			}
		}
	}

	if(pINI->ReadString(section, "ParaDrop.Num", "", Ares::readBuffer, Ares::readLength)) {
		this->ParaDropNum.Clear();
		this->ParaDropFallbackNum = nullptr;

		char* context = nullptr;
		for(p = strtok_s(Ares::readBuffer, Ares::readDelims, &context); p && *p; p = strtok_s(nullptr, Ares::readDelims, &context)) {
			this->ParaDropNum.AddItem(atoi(p));
		}
	}

	this->SidebarMixFileIndex =  pINI->ReadInteger(section, "Sidebar.MixFileIndex", this->SidebarMixFileIndex);
	this->SidebarYuriFileNames = pINI->ReadBool(section, "Sidebar.YuriFileNames", this->SidebarYuriFileNames);
	this->ToolTipTextColor.Read(&exINI, section, "ToolTipColor");
	this->SurvivorDivisor.Read(&exINI, section, "SurvivorDivisor");

	if(pINI->ReadString(section, "MessageTextColor", "", Ares::readBuffer, 0x80)) {
		if(ColorScheme* pCS = ColorScheme::Find(Ares::readBuffer)) {
			this->MessageTextColorIndex = pCS->ArrayIndex;
		}
	}
}

int SideExt::ExtData::GetSurvivorDivisor() const {
	if(this->SurvivorDivisor.isset()) {
		return this->SurvivorDivisor;
	}

	return this->GetDefaultSurvivorDivisor();
}

int SideExt::ExtData::GetDefaultSurvivorDivisor() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedSurvivorDivisor;
	case 1:
		return RulesClass::Instance->SovietSurvivorDivisor;
	case 2:
		return RulesClass::Instance->ThirdSurvivorDivisor;
	default:
		//return 0; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedSurvivorDivisor;
	}
}

InfantryTypeClass* SideExt::ExtData::GetCrew() const {
	if(this->Crew.isset()) {
		return this->Crew;
	}

	return this->GetDefaultCrew();
}

InfantryTypeClass* SideExt::ExtData::GetDefaultCrew() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedCrew;
	case 1:
		return RulesClass::Instance->SovietCrew;
	case 2:
		return RulesClass::Instance->ThirdCrew;
	default:
		//return RulesClass::Instance->Technician; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedCrew;
	}
}

InfantryTypeClass* SideExt::ExtData::GetEngineer() const {
	return this->Engineer.Get(RulesClass::Instance->Engineer);
}

InfantryTypeClass* SideExt::ExtData::GetTechnician() const {
	return this->Technician.Get(RulesClass::Instance->Technician);
}

InfantryTypeClass* SideExt::ExtData::GetDisguise() const {
	if(this->Disguise.isset()) {
		return this->Disguise;
	}

	return this->GetDefaultDisguise();
}

InfantryTypeClass* SideExt::ExtData::GetDefaultDisguise() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedDisguise;
	case 1:
		return RulesClass::Instance->SovietDisguise;
	case 2:
		return RulesClass::Instance->ThirdDisguise;
	default:
		//return RulesClass::Instance->ThirdDisguise; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedDisguise;
	}
}

Iterator<int> SideExt::ExtData::GetBaseDefenseCounts() const {
	if(this->BaseDefenseCounts.HasValue()) {
		return this->BaseDefenseCounts;
	}

	return this->GetDefaultBaseDefenseCounts();
}

Iterator<int> SideExt::ExtData::GetDefaultBaseDefenseCounts() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	case 1:
		return RulesClass::Instance->SovietBaseDefenseCounts;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenseCounts;
	default:
		//return Iterator<int>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	}
}

Iterator<BuildingTypeClass*> SideExt::ExtData::GetBaseDefenses() const {
	if(this->BaseDefenses.HasValue()) {
		return this->BaseDefenses;
	}

	return this->GetDefaultBaseDefenses();
}

Iterator<BuildingTypeClass*> SideExt::ExtData::GetDefaultBaseDefenses() const {
	switch(this->ArrayIndex) {
	case 0:
		return RulesClass::Instance->AlliedBaseDefenses;
	case 1:
		return RulesClass::Instance->SovietBaseDefenses;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenses;
	default:
		//return Iterator<BuildingTypeClass*>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenses;
	}
}

DWORD SideExt::LoadTextColor(REGISTERS* R, DWORD dwReturnAddress)
{
	// if there is a cached LoadTextColor, use that.
	int index = SideExt::CurrentLoadTextColor;
	if(auto pCS = ColorScheme::Array->GetItemOrDefault(index)) {
		R->EAX(pCS);
		return dwReturnAddress;
	}

	return 0;
}

DWORD SideExt::MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	GET(ScenarioClass *, pScen, EAX); //TODO test

	SideClass* pSide = SideClass::Array->GetItem(pScen->PlayerSideIndex);
	if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
		return pData->SidebarYuriFileNames
			? dwReturnAddress1
			: dwReturnAddress2
		;
	} else {
		return 0;
	}
}

// =============================
// load/save

bool Container<SideExt>::Save(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
		//pData->BaseDefenses.Save(pStm);
		//pData->BaseDefenseCounts.Save(pStm);
		pData->ParaDrop.Save(pStm);
		pData->ParaDropNum.Save(pStm);
	}

	return pData != nullptr;
}

bool Container<SideExt>::Load(SideClass *pThis, IStream *pStm) {
	SideExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->Disguise);
	SWIZZLE(pData->Crew);
	//pData->BaseDefenses.Load(pStm, 1);
	//pData->BaseDefenseCounts.Load(pStm, 0);
	pData->ParaDrop.Load(pStm, 1);
	pData->ParaDropNum.Load(pStm, 0);

	return pData != nullptr;
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

DEFINE_HOOK_AGAIN(6A48A0, SideClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(6A4780, SideClass_SaveLoad_Prefix, 6)
{
	GET_STACK(SideExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<SideExt>::PrepareStream(pItem, pStm);

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

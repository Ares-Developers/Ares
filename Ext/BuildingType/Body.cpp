#include "Body.h"
#include "..\TechnoType\Body.h"
#include "..\House\Body.h"

const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
Container<BuildingTypeExt> BuildingTypeExt::ExtMap;

// =============================
// member funcs

void BuildingTypeExt::ExtData::Initialize(BuildingTypeClass *pThis) {
	if(pThis->SecretLab) {
		this->Secret_Boons.Clear();
		DynamicVectorClass<TechnoTypeClass *> *Options
			= (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretInfantry();
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}

		Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretUnits();
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}

		Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretBuildings();
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}
	}
	this->_Initialized = is_Inited;
}

void Container<BuildingTypeExt>::Save(BuildingTypeClass *pThis, IStream *pStm) {
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(!pData->CustomData) {
		pData->IsCustom = true;
	}

	ULONG out;
	pStm->Write(pData, pData->Size(), &out);

	//if there's custom data, write it
	if(pData->IsCustom) {
		pStm->Write(
			pData->CustomData,
			sizeof(CellStruct) * (pData->CustomWidth * pData->CustomHeight + 1),
			&out);
	}
};

void Container<BuildingTypeExt>::Load(BuildingTypeClass *pThis, IStream *pStm) {
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.FindOrAllocate(pThis);

	ULONG out;
	pStm->Read(pData, pData->Size(), &out);

	//if there's custom data, read it
	if(pData->IsCustom && pData->CustomWidth > 0 && pData->CustomHeight > 0) {
		pData->CustomData = new CellStruct[pData->CustomWidth * pData->CustomHeight + 1];

		pStm->Read(
			pData->CustomData,
			sizeof(CellStruct) * (pData->CustomWidth * pData->CustomHeight + 1),
			&out);

		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(pData->CustomData);
	}

#ifdef DEBUGBUILD
	assert(this->SavedCanary == Extension<BuildingTypeClass>::Canary);
#endif
}; 

void BuildingTypeExt::ExtData::LoadFromINI(BuildingTypeClass *pThis, CCINIClass* pINI)
{
	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	char* pArtID = pThis->get_ImageFile();
	char* pID = pThis->get_ID();

	this->Firewall_Is = pINI->ReadBool(pID, "Firestorm.Wall", this->Firewall_Is);

	this->Solid_Height = CCINIClass::INI_Art->ReadInteger(pArtID, "SolidHeight", this->Solid_Height);
	CCINIClass* pArtINI = CCINIClass::INI_Art;

	if(this->IsCustom) {
		//Reset
		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(this->CustomData);
	} else if(pArtINI) {

		char str[0x80]="\0";

		if(pArtINI->ReadString(pArtID, "Foundation", "", str, 0x80) && !_strcmpi(str,"Custom")) {
			//Custom Foundation!
			this->IsCustom = true;
			pThis->set_Foundation(FOUNDATION_CUSTOM);

			//Load Width and Height
			this->CustomWidth = pArtINI->ReadInteger(pArtID, "Foundation.X", 0);
			this->CustomHeight = pArtINI->ReadInteger(pArtID, "Foundation.Y", 0);

			//Allocate CellStruct array
			if(this->CustomData) {
				delete this->CustomData;
			}

			CellStruct* pFoundationData = new CellStruct[this->CustomWidth * this->CustomHeight + 1];
			
			this->CustomData = pFoundationData;
			pThis->set_FoundationData(pFoundationData);

			//Load FoundationData
			CellStruct* pCurrent = pFoundationData;
			char key[0x20];

			for(int i = 0; i < this->CustomWidth * this->CustomHeight; i++) {
				_snprintf(key, 32, "Foundation.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					short x, y;
					if(sscanf(str, "%d,%d", &x, &y) == 2) {
						pCurrent->X = x;
						pCurrent->Y = y;
						++pCurrent;
					}
				} else {
					//Set end vector
					pCurrent->X = 0x7FFF;
					pCurrent->Y = 0x7FFF;
					break;
				}
			}
		}
	}


	// secret lab
	if(pINI->ReadString(pThis->get_ID(), "SecretLab.PossibleBoons", "", Ares::readBuffer, Ares::readLength)) {
		this->Secret_Boons.Clear();
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			TechnoTypeClass *pTechno = TechnoTypeClass::Find(cur);
			if(pTechno) {
				this->Secret_Boons.AddItem(pTechno);
			}
		}
	}
	this->Secret_RecalcOnCapture =
		pINI->ReadBool(pThis->get_ID(), "SecretLab.GenerateOnCapture", this->Secret_RecalcOnCapture);

	this->_Initialized = is_Completed;
}

void BuildingTypeExt::UpdateSecretLabOptions(BuildingClass *pThis)
{
	BuildingTypeClass *pType = pThis->Type;
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pType);

	DEBUGLOG("Secret Lab update for %s\n", pType->get_ID());

	if(!pData->Secret_Boons.Count || (pData->Secret_Placed && !pData->Secret_RecalcOnCapture)) {
		return;
	}

	TechnoTypeClass *Result = pType->SecretInfantry;
	if(!Result) {
		Result = pType->SecretUnit;
		if(!Result) {
			Result = pType->SecretBuilding;
		}
	}
	if(Result) {
		pThis->SecretProduction = Result;
		return;
	}

	HouseClass *Owner = pThis->Owner;
	int OwnerBits = 1 << Owner->Type->ArrayIndex;

	DynamicVectorClass<TechnoTypeClass *> Options;
	for(int i = 0; i < pData->Secret_Boons.Count; ++i) {
		TechnoTypeClass * Option = pData->Secret_Boons.GetItem(i);
		TechnoTypeExt::ExtData* pTech = TechnoTypeExt::ExtMap.Find(Option);

		if(pTech->Secret_RequiredHouses & OwnerBits && !(pTech->Secret_ForbiddenHouses & OwnerBits)) {
			if(!HouseExt::RequirementsMet(Owner, Option)) {
				Options.AddItem(Option);
			}
		}
	}

	if(Options.Count < 1) {
		DEBUGLOG("Secret Lab [%s] has no boons applicable to country [%s]!\n",
			pType->get_ID(), Owner->Type->get_ID());
		return;
	}

	int idx = Randomizer::Global()->RandomRanged(0, Options.Count - 1);
	Result = Options[idx];

	DEBUGLOG("Secret Lab rolled %s for %s\n", Result->get_ID(), pType->get_ID());
	pData->Secret_Placed = true;
	pThis->set_SecretProduction(Result);
}

// =============================
// container hooks

DEFINE_HOOK(45E50C, BuildingTypeClass_CTOR, 6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}


DEFINE_HOOK(45E580, BuildingTypeClass_DTOR, 5)
{
	GET(BuildingTypeClass*, pItem, ECX);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(4652ED, BuildingTypeClass_Load, 7)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x20);
	GET_STACK(IStream*, pStm, 0x24);

	BuildingTypeExt::ExtMap.Load(pItem, pStm);
	return 0;
}

DEFINE_HOOK(46536A, BuildingTypeClass_Save, 7)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x14);
	GET_STACK(IStream*, pStm, 0x18);

	BuildingTypeExt::ExtMap.Save(pItem, pStm);
	return 0;
}

DEFINE_HOOK(464A49, BuildingTypeClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(464A56, BuildingTypeClass_LoadFromINI, A)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

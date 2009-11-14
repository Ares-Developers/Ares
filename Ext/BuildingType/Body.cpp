#include "Body.h"
#include "..\TechnoType\Body.h"
#include "..\House\Body.h"

const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
Container<BuildingTypeExt> BuildingTypeExt::ExtMap;

BuildingTypeExt::TT *Container<BuildingTypeExt>::SavingObject = NULL;
IStream *Container<BuildingTypeExt>::SavingStream = NULL;

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

void BuildingTypeExt::ExtData::LoadFromINIFile(BuildingTypeClass *pThis, CCINIClass* pINI)
{
	char* pArtID = pThis->get_ImageFile();
	char* pID = pThis->get_ID();

	this->Firewall_Is = pINI->ReadBool(pID, "Firestorm.Wall", this->Firewall_Is);

	CCINIClass* pArtINI = CCINIClass::INI_Art;
	this->Solid_Height = pArtINI->ReadInteger(pArtID, "SolidHeight", this->Solid_Height);

	if(this->IsCustom) {
		//Reset
		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(this->CustomData);
	} else if(pArtINI) {

		char str[0x80]="\0";

		if(pArtINI->ReadString(pArtID, "Foundation", "", str, 0x80) && !_strcmpi(str,"Custom")) {
			//Custom Foundation!
			this->IsCustom = true;
			pThis->Foundation = FOUNDATION_CUSTOM;

			//Load Width and Height
			this->CustomWidth = pArtINI->ReadInteger(pArtID, "Foundation.X", 0);
			this->CustomHeight = pArtINI->ReadInteger(pArtID, "Foundation.Y", 0);
			this->OutlineLength = pArtINI->ReadInteger(pArtID, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if(this->OutlineLength < 10) {
				this->OutlineLength = 10;
			}

			//Allocate CellStruct array
			if(this->CustomData) {
				delete[] this->CustomData;
			}

			if(this->OutlineData) {
				delete[] this->OutlineData;
			}

			CellStruct* pFoundationData = new CellStruct[this->CustomWidth * this->CustomHeight + 1];
			CellStruct* pOutlineData = new CellStruct[this->OutlineLength + 1];

			this->CustomData = pFoundationData;
			this->OutlineData = pOutlineData;
			pThis->FoundationData = pFoundationData;
			pThis->FoundationOutside = pOutlineData;

			//Load FoundationData
			CellStruct* pCurrent = pFoundationData;
			char key[0x20];

			for(int i = 0; i < this->CustomWidth * this->CustomHeight; ++i) {
				_snprintf(key, 32, "Foundation.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					short x = 0, y = 0;
					sscanf(str, "%d,%d", &x, &y);
					pCurrent->X = x;
					pCurrent->Y = y;
					++pCurrent;
				} else {
					break;
				}
			}

			//Set end vector
			pCurrent->X = 0x7FFF;
			pCurrent->Y = 0x7FFF;

			pCurrent = pOutlineData;
			for(int i = 0; i < this->OutlineLength; ++i) {
				_snprintf(key, 32, "FoundationOutline.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					short x = 0, y = 0;
					sscanf(str, "%d,%d", &x, &y);
					pCurrent->X = x;
					pCurrent->Y = y;
					++pCurrent;
				} else {
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					pCurrent->X = 0x7FFF;
					pCurrent->Y = 0x7FFF;
					++pCurrent;
				}
			}

			//Set end vector
			pCurrent->X = 0x7FFF;
			pCurrent->Y = 0x7FFF;
		}

	}

	if(pINI->ReadString(pThis->get_ID(), "SecretLab.PossibleBoons", "", Ares::readBuffer, Ares::readLength)) {
		this->Secret_Boons.Clear();
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			TechnoTypeClass *pTechno = TechnoTypeClass::Find(cur);
			if(pTechno) {
				this->Secret_Boons.AddItem(pTechno);
			}
		}
	}

	this->Secret_RecalcOnCapture = pINI->ReadBool(pThis->get_ID(), "SecretLab.GenerateOnCapture", this->Secret_RecalcOnCapture);

	// added on 11.11.09 for #221 and children (Trenches)
	this->UCPassThrough = pINI->ReadDouble(pID, "UC.PassThrough", this->UCPassThrough);
	this->UCFatalRate = pINI->ReadDouble(pID, "UC.FatalRate", this->UCFatalRate);
	this->UCDamageMultiplier = pINI->ReadDouble(pID, "UC.DamageMultiplier", this->UCDamageMultiplier);
	this->BunkerRaidable = pINI->ReadBool(pID, "Bunker.Raidable", this->BunkerRaidable);
	if(pINI->ReadString(pID, "IsTrench", "", Ares::readBuffer, Ares::readLength)) {
		/*  If the list of kinds is empty so far, just add this kind as the first one;
			if there already are kinds in it, compare the current kind against the kinds in the list;
			if it was found, assign that kind's ID to this type;
			if it wasn't found, add this kind at the end of the list and assign the ID.

			I originally thought of using a map here, but I figured the probability that the kinds list
			grows so long that the search through all kinds takes up significant time is very low, and
			vectors are far simpler to use in this situation.
		*/
		if(this->trenchKinds.size()) {
			signed int foundMatch = -1;
			for(unsigned int i = 0; i < this->trenchKinds.size(); ++i) {
				if(this->trenchKinds.at(i).compare(Ares::readBuffer) == 0) {
					foundMatch = i;
					break;
				}
			}

			if(foundMatch > -1) {
				this->IsTrench = foundMatch;
			} else {
				this->IsTrench = this->trenchKinds.size();
				this->trenchKinds.push_back(Ares::readBuffer);
			}

		} else {
			this->IsTrench = 0;
			this->trenchKinds.push_back(Ares::readBuffer);
		}
	}
	if(pINI->ReadString(pID, "Rubble.Intact", "", Ares::readBuffer, Ares::readLength)) {
		this->RubbleIntact = BuildingTypeClass::Find(Ares::readBuffer);
	}
	if(pINI->ReadString(pID, "Rubble.Destroyed", "", Ares::readBuffer, Ares::readLength)) {
		this->RubbleDestroyed = BuildingTypeClass::Find(Ares::readBuffer);
	}

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
// load/save

void Container<BuildingTypeExt>::Save(BuildingTypeClass *pThis, IStream *pStm) {
	BuildingTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData && pData->IsCustom) {
		ULONG out;
		pStm->Write(pData->CustomData,
			sizeof(CellStruct) * (pData->CustomWidth * pData->CustomHeight + 1),
			&out);
		pStm->Write(pData->OutlineData,
			sizeof(CellStruct) * (pData->OutlineLength + 1),
			&out);
	}
};

void Container<BuildingTypeExt>::Load(BuildingTypeClass *pThis, IStream *pStm) {
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);
//	this->FindOrAllocate(pThis);

	ULONG out;

	//if there's custom data, read it
	if(pData->IsCustom && pData->CustomWidth > 0 && pData->CustomHeight > 0) {
		pData->CustomData = new CellStruct[pData->CustomWidth * pData->CustomHeight + 1];
		pData->OutlineData = new CellStruct[pData->OutlineLength + 1];

		pStm->Read(
			pData->CustomData,
			sizeof(CellStruct) * (pData->CustomWidth * pData->CustomHeight + 1),
			&out);

		pStm->Read(
			pData->OutlineData,
			sizeof(CellStruct) * (pData->OutlineLength + 1),
			&out);

		pThis->Foundation = FOUNDATION_CUSTOM;
		pThis->FoundationData = pData->CustomData;
		pThis->FoundationOutside = pData->OutlineData;
	} else {
		pData->CustomData = pData->OutlineData = NULL;
	}

#ifdef DEBUGBUILD
	assert(this->SavedCanary == Extension<BuildingTypeClass>::Canary);
#endif
};

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

DEFINE_HOOK(465010, BuildingTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(465300, BuildingTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BuildingTypeExt>::SavingObject = pItem;
	Container<BuildingTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(4652ED, BuildingTypeClass_Load_Suffix, 7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(46536A, BuildingTypeClass_Save_Suffix, 7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
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

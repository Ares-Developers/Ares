#include "Body.h"
#include "../TechnoType/Body.h"
#include "../House/Body.h"

#include <InfantryClass.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
Container<BuildingTypeExt> BuildingTypeExt::ExtMap;

template<> BuildingTypeExt::TT *Container<BuildingTypeExt>::SavingObject = NULL;
template<> IStream *Container<BuildingTypeExt>::SavingStream = NULL;

std::vector<std::string> BuildingTypeExt::ExtData::trenchKinds;

// =============================
// member funcs

void BuildingTypeExt::ExtData::Initialize(BuildingTypeClass *pThis)
{
	if(pThis->SecretLab) {
		this->Secret_Boons.Clear();
		DynamicVectorClass<TechnoTypeClass *> *Options
			= (DynamicVectorClass<TechnoTypeClass *> *)&RulesClass::Instance->SecretInfantry;
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}

		Options = (DynamicVectorClass<TechnoTypeClass *> *)&RulesClass::Instance->SecretUnits;
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}

		Options = (DynamicVectorClass<TechnoTypeClass *> *)&RulesClass::Instance->SecretBuildings;
		for(int i = 0; i < Options->Count; ++i) {
			this->Secret_Boons.AddItem(Options->GetItem(i));
		}
	}
	this->PrismForwarding.Initialize(pThis);
}

void BuildingTypeExt::ExtData::LoadFromINIFile(BuildingTypeClass *pThis, CCINIClass* pINI)
{
	char* pArtID = pThis->ImageFile;
	char* pID = pThis->ID;

	this->PrismForwarding.LoadFromINIFile(pThis, pINI);

	if(pThis->UnitRepair && pThis->Factory == abs_AircraftType) {
		Debug::FatalErrorAndExit(
			"BuildingType [%s] has both UnitRepair=yes and Factory=AircraftType.\n"
			"This combination causes Internal Errors and other unwanted behaviour.", pID);
	}

	this->Firewall_Is = pINI->ReadBool(pID, "Firestorm.Wall", this->Firewall_Is);

	CCINIClass* pArtINI = CCINIClass::INI_Art;
	this->Solid_Height = pArtINI->ReadInteger(pArtID, "SolidHeight", this->Solid_Height);

	if(this->IsCustom) {
		//Reset
		pThis->Foundation = FOUNDATION_CUSTOM;
		pThis->FoundationData = this->CustomData;
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
				delete [] this->CustomData;
			}

			if(this->OutlineData) {
				delete [] this->OutlineData;
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

			auto ParsePoint = [](CellStruct* &pCell, const char* str) -> void {
				int x = 0, y = 0;
				switch(sscanf(str, "%d,%d", &x, &y)) {
				case 0:
					x = 0;
					// fallthrough
				case 1:
					y = 0;
				}
				pCell->X = (short)x;
				pCell->Y = (short)y;
				++pCell;
			};

			for(int i = 0; i < this->CustomWidth * this->CustomHeight; ++i) {
				_snprintf(key, 31, "Foundation.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					ParsePoint(pCurrent, str);
				} else {
					break;
				}
			}

			//Set end vector
			pCurrent->X = 0x7FFF;
			pCurrent->Y = 0x7FFF;

			pCurrent = pOutlineData;
			for(int i = 0; i < this->OutlineLength; ++i) {
				_snprintf(key, 31, "FoundationOutline.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					ParsePoint(pCurrent, str);
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

	if(pINI->ReadString(pID, "SecretLab.PossibleBoons", "", Ares::readBuffer, Ares::readLength)) {
		this->Secret_Boons.Clear();
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
			TechnoTypeClass *pTechno = TechnoTypeClass::Find(cur);
			if(pTechno) {
				this->Secret_Boons.AddItem(pTechno);
			} else {
				Debug::INIParseFailed(pID, "SecretLab.PossibleBoons", cur);
			}
		}
	}

	this->Secret_RecalcOnCapture = pINI->ReadBool(pID, "SecretLab.GenerateOnCapture", this->Secret_RecalcOnCapture);

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
		if(trenchKinds.size()) {
			signed int foundMatch = -1;
			for(unsigned int i = 0; i < trenchKinds.size(); ++i) {
				if(trenchKinds.at(i).compare(Ares::readBuffer) == 0) {
					foundMatch = i;
					break;
				}
			}

			if(foundMatch > -1) {
				this->IsTrench = foundMatch;
			} else {
				this->IsTrench = trenchKinds.size();
				trenchKinds.push_back(Ares::readBuffer);
			}

		} else {
			this->IsTrench = 0;
			trenchKinds.push_back(Ares::readBuffer);
		}
	}
	if(pINI->ReadString(pID, "Rubble.Intact", "", Ares::readBuffer, Ares::readLength)) {
		this->RubbleIntact = BuildingTypeClass::Find(Ares::readBuffer);
		if(!this->RubbleIntact && VALIDTAG(Ares::readBuffer)) {
			Debug::INIParseFailed(pID, "Rubble.Intact", Ares::readBuffer);
		}
	}
	if(pINI->ReadString(pID, "Rubble.Destroyed", "", Ares::readBuffer, Ares::readLength)) {
		this->RubbleDestroyed = BuildingTypeClass::Find(Ares::readBuffer);
		if(this->RubbleDestroyed) {
			this->RubbleDestroyed->Capturable = false;
			this->RubbleDestroyed->TogglePower = false;
			this->RubbleDestroyed->Unsellable = true;
			this->RubbleDestroyed->CanBeOccupied = false;
		} else if(VALIDTAG(Ares::readBuffer)) {
			Debug::INIParseFailed(pID, "Rubble.Destroyed", Ares::readBuffer);
		}
	}

	this->LightningRod_Modifier = pINI->ReadDouble(pID, "LightningRod.Modifier", this->LightningRod_Modifier);

//	this->LegacyRadarEffect = pINI->ReadBool(pID, "SpyEffect.LegacyRadar", this->LegacyRadarEffect);
//	this->DisplayProduction = pINI->ReadBool(pID, "SpyEffect.DisplayProduction", this->DisplayProduction);

	INI_EX exINI(pINI);
	this->InfiltrateCustom.Read(&exINI, pID, "SpyEffect.Custom");
	this->RevealProduction.Read(&exINI, pID, "SpyEffect.RevealProduction");
	this->ResetSW.Read(&exINI, pID, "SpyEffect.ResetSuperweapons");
	this->ResetRadar.Read(&exINI, pID, "SpyEffect.ResetRadar");
	this->RevealRadar.Read(&exINI, pID, "SpyEffect.RevealRadar");
	this->GainVeterancy.Read(&exINI, pID, "SpyEffect.UnitVeterancy");
	this->StolenTechIndex.Read(&exINI, pID, "SpyEffect.StolenTechIndex");
	this->PowerOutageDuration.Read(&exINI, pID, "SpyEffect.PowerOutageDuration");
	this->StolenMoneyAmount.Read(&exINI, pID, "SpyEffect.StolenMoneyAmount");
	this->StolenMoneyPercentage.Read(&exINI, pID, "SpyEffect.StolenMoneyPercentage");
	this->UnReverseEngineer.Read(&exINI, pID, "SpyEffect.UndoReverseEngineer");

	if(this->StolenTechIndex >= 32) {
		Debug::DevLog(Debug::Warning, "BuildingType %s has a SpyEffect.StolenTechIndex of %d. The value has to be less than 32.\n", pID, this->StolenTechIndex.Get());
		this->StolenTechIndex = -1;
	}

	// #218 Specific Occupiers
	this->AllowedOccupiers.Read(&exINI, pID, "CanBeOccupiedBy");
	if(!this->AllowedOccupiers.empty()) {
		// having a specific occupier list implies that this building is supposed to be occupiable
		pThis->CanBeOccupied = true;
	}

	this->ReverseEngineersVictims.Read(&exINI, pID, "ReverseEngineersVictims");

	this->CloningFacility.Read(&exINI, pID, "CloningFacility");
	this->Factory_ExplicitOnly.Read(&exINI, pID, "Factory.ExplicitOnly");
}

void BuildingTypeExt::ExtData::CompleteInitialization(BuildingTypeClass *pThis) {
	// enforce same foundations for rubble/intact building pairs
	if(this->RubbleDestroyed && !BuildingTypeExt::IsFoundationEqual(this->AttachedToObject, this->RubbleDestroyed)) {
		Debug::FatalErrorAndExit("BuildingType %s and its Rubble.Destroyed %s don't have the same foundation.", this->AttachedToObject->ID, this->RubbleDestroyed->ID);
	}
	if(this->RubbleIntact && !BuildingTypeExt::IsFoundationEqual(this->AttachedToObject, this->RubbleIntact)) {
		Debug::FatalErrorAndExit("BuildingType %s and its Rubble.Intact %s don't have the same foundation.", this->AttachedToObject->ID, this->RubbleIntact->ID);
	}
}

void BuildingTypeExt::UpdateSecretLabOptions(BuildingClass *pThis)
{
	BuildingTypeClass *pType = pThis->Type;
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pType);

	DEBUGLOG("Secret Lab update for %s\n", pType->get_ID());

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

	if(!pData->Secret_Boons.Count || (pData->Secret_Placed && !pData->Secret_RecalcOnCapture)) {
		return;
	}

	HouseClass *Owner = pThis->Owner;
	unsigned int OwnerBits = 1 << Owner->Type->ArrayIndex;

	DynamicVectorClass<TechnoTypeClass *> Options;
	for(int i = 0; i < pData->Secret_Boons.Count; ++i) {
		TechnoTypeClass * Option = pData->Secret_Boons.GetItem(i);
		TechnoTypeExt::ExtData* pTech = TechnoTypeExt::ExtMap.Find(Option);

		if((pTech->Secret_RequiredHouses & OwnerBits) && !(pTech->Secret_ForbiddenHouses & OwnerBits)) {
			bool ShouldAdd = false;
			switch(HouseExt::RequirementsMet(Owner, Option)) {
				case HouseExt::Forbidden:
				case HouseExt::Incomplete:
					ShouldAdd = true;
					break;
			}
			if(ShouldAdd) {
				Options.AddItem(Option);
			}
		}
	}

	if(Options.Count < 1) {
		DEBUGLOG("Secret Lab [%s] has no boons applicable to country [%s]!\n",
			pType->ID, Owner->Type->ID);
		return;
	}

	int idx = ScenarioClass::Instance->Random.RandomRanged(0, Options.Count - 1);
	Result = Options[idx];

	DEBUGLOG("Secret Lab rolled %s for %s\n", Result->ID, pType->ID);
	pData->Secret_Placed = true;
	pThis->SecretProduction = Result;
}

// Naive function to return whether two buildings have tha same foundation.
bool BuildingTypeExt::IsFoundationEqual(BuildingTypeClass *pTBldA, BuildingTypeClass *pTBldB) {
	if(pTBldA && pTBldB) {
		// must have same foundation id
		if(pTBldA->Foundation != pTBldB->Foundation) {
			return false;
		}

		// non-custom foundations need no special handling
		if(pTBldA->Foundation != FOUNDATION_CUSTOM) {
			return true;
		}

		// custom foundation
		BuildingTypeExt::ExtData *pDataA = BuildingTypeExt::ExtMap.Find(pTBldA);
		BuildingTypeExt::ExtData *pDataB = BuildingTypeExt::ExtMap.Find(pTBldB);
		if(pDataA->CustomWidth == pDataB->CustomWidth) {
			if(pDataA->CustomHeight == pDataB->CustomHeight) {
				// compare unsorted arrays the hard way: O(n²)
				int length = (pDataA->CustomHeight * pDataA->CustomHeight + 1);
				for(int i=0; i<length; ++i) {
					bool found = false;
					for(int j=0; j<length; ++j) {
						if((pDataA->CustomData[i].X == pDataB->CustomData[j].X) && (pDataA->CustomData[i].Y == pDataB->CustomData[j].Y)) {
							found = true;
							break;
						}
					}
					if(!found) {
						return false;
					}
				}
				// found everyting.
				return true;
			}
		}
	}
	return false;
}

// Short check: Is the building of a linkable kind at all?
bool BuildingTypeExt::ExtData::IsLinkable() {
	return this->Firewall_Is || (this->IsTrench > -1);
}

bool BuildingTypeExt::ExtData::CanBeOccupiedBy(InfantryClass *whom) {
	// if CanBeOccupiedBy isn't empty, we have to check if this soldier is allowed in
	return this->AllowedOccupiers.empty() || (this->AllowedOccupiers == whom->Type);
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
		pData->OutlineData = new CellStruct [pData->OutlineLength + 1];

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


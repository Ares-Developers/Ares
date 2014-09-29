#include "Body.h"
#include "../TechnoType/Body.h"
#include "../House/Body.h"
#include "../../Utilities/TemplateDef.h"

#include <InfantryClass.h>
#include <VocClass.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
Container<BuildingTypeExt> BuildingTypeExt::ExtMap;

template<> BuildingTypeExt::TT *Container<BuildingTypeExt>::SavingObject = nullptr;
template<> IStream *Container<BuildingTypeExt>::SavingStream = nullptr;

std::vector<std::string> BuildingTypeExt::ExtData::trenchKinds;

// =============================
// member funcs

void BuildingTypeExt::ExtData::Initialize()
{
	if(this->OwnerObject()->SecretLab) {
		this->Secret_Boons.Clear();
		for(auto pType : RulesClass::Instance->SecretInfantry) {
			this->Secret_Boons.AddItem(pType);
		}

		for(auto pType : RulesClass::Instance->SecretUnits) {
			this->Secret_Boons.AddItem(pType);
		}

		for(auto pType : RulesClass::Instance->SecretBuildings) {
			this->Secret_Boons.AddItem(pType);
		}
	}
	this->PrismForwarding.Initialize(this->OwnerObject());
}

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	char* pArtID = pThis->ImageFile;
	char* pID = pThis->ID;

	this->PrismForwarding.LoadFromINIFile(pThis, pINI);

	if(pThis->UnitRepair && pThis->Factory == AbstractType::AircraftType) {
		Debug::FatalErrorAndExit(
			"BuildingType [%s] has both UnitRepair=yes and Factory=AircraftType.\n"
			"This combination causes Internal Errors and other unwanted behaviour.", pID);
	}

	this->Firewall_Is = pINI->ReadBool(pID, "Firestorm.Wall", this->Firewall_Is);

	CCINIClass* pArtINI = CCINIClass::INI_Art;
	this->Solid_Height = pArtINI->ReadInteger(pArtID, "SolidHeight", this->Solid_Height);

	if(this->IsCustom) {
		//Reset
		pThis->Foundation = BuildingTypeExt::CustomFoundation;
		pThis->FoundationData = this->CustomData.data();
	} else if(pArtINI) {

		char str[0x80]="\0";

		if(pArtINI->ReadString(pArtID, "Foundation", "", str, 0x80) && !_strcmpi(str,"Custom")) {
			//Custom Foundation!
			this->IsCustom = true;
			pThis->Foundation = BuildingTypeExt::CustomFoundation;

			//Load Width and Height
			this->CustomWidth = pArtINI->ReadInteger(pArtID, "Foundation.X", 0);
			this->CustomHeight = pArtINI->ReadInteger(pArtID, "Foundation.Y", 0);
			this->OutlineLength = pArtINI->ReadInteger(pArtID, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if(this->OutlineLength < 10) {
				this->OutlineLength = 10;
			}

			//Allocate CellStruct array
			this->CustomData.assign(this->CustomWidth * this->CustomHeight + 1, CellStruct::Empty);
			this->OutlineData.assign(this->OutlineLength + 1, CellStruct::Empty);

			pThis->FoundationData = this->CustomData.data();
			pThis->FoundationOutside = this->OutlineData.data();

			//Load FoundationData
			CellStruct* pCurrent = this->CustomData.data();
			char key[0x20];

			auto ParsePoint = [](CellStruct* &pCell, const char* str) -> void {
				int x = 0, y = 0;
				switch(sscanf_s(str, "%d,%d", &x, &y)) {
				case 0:
					x = 0;
					// fallthrough
				case 1:
					y = 0;
				}
				pCell->X = static_cast<short>(x);
				pCell->Y = static_cast<short>(y);
				++pCell;
			};

			for(int i = 0; i < this->CustomWidth * this->CustomHeight; ++i) {
				_snprintf_s(key, _TRUNCATE, "Foundation.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str, 0x80)) {
					ParsePoint(pCurrent, str);
				} else {
					break;
				}
			}

			//Set end vector
			pCurrent->X = 0x7FFF;
			pCurrent->Y = 0x7FFF;

			pCurrent = this->OutlineData.data();
			for(int i = 0; i < this->OutlineLength; ++i) {
				_snprintf_s(key, _TRUNCATE, "FoundationOutline.%d", i);
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

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
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
		/*  Find the name in the list of kinds; if the list is empty, distance is 0, if the item isn't in
			the list, the index is the current list's size(); if the returned iterator is beyond the list,
			add the name to the list, which makes the previously calculated index (th distance) valid.
			(changed by AlexB 2014-01-16)

			I originally thought of using a map here, but I figured the probability that the kinds list
			grows so long that the search through all kinds takes up significant time is very low, and
			vectors are far simpler to use in this situation.
		*/
		auto it = std::find(trenchKinds.begin(), trenchKinds.end(), Ares::readBuffer);
		this->IsTrench = std::distance(trenchKinds.begin(), it);
		if(it == trenchKinds.end()) {
			trenchKinds.push_back(Ares::readBuffer);
		}
	}

	this->LightningRod_Modifier = pINI->ReadDouble(pID, "LightningRod.Modifier", this->LightningRod_Modifier);

//	this->LegacyRadarEffect = pINI->ReadBool(pID, "SpyEffect.LegacyRadar", this->LegacyRadarEffect);
//	this->DisplayProduction = pINI->ReadBool(pID, "SpyEffect.DisplayProduction", this->DisplayProduction);

	INI_EX exINI(pINI);

	this->RubbleDestroyed.Read(exINI, pID, "Rubble.Destroyed");
	this->RubbleIntact.Read(exINI, pID, "Rubble.Intact");
	this->RubbleDestroyedAnim.Read(exINI, pID, "Rubble.Destroyed.Anim");
	this->RubbleIntactAnim.Read(exINI, pID, "Rubble.Intact.Anim");
	this->RubbleDestroyedOwner.Read(exINI, pID, "Rubble.Destroyed.Owner");
	this->RubbleIntactOwner.Read(exINI, pID, "Rubble.Intact.Owner");
	this->RubbleDestroyedStrength.Read(exINI, pID, "Rubble.Destroyed.Strength");
	this->RubbleIntactStrength.Read(exINI, pID, "Rubble.Intact.Strength");
	this->RubbleDestroyedRemove.Read(exINI, pID, "Rubble.Destroyed.Remove");
	this->RubbleIntactRemove.Read(exINI, pID, "Rubble.Intact.Remove");

	if(this->RubbleDestroyed) {
		this->RubbleDestroyed->Capturable = false;
		this->RubbleDestroyed->TogglePower = false;
		this->RubbleDestroyed->Unsellable = true;
		this->RubbleDestroyed->CanBeOccupied = false;
	}

	this->InfiltrateCustom.Read(exINI, pID, "SpyEffect.Custom");
	this->RevealProduction.Read(exINI, pID, "SpyEffect.RevealProduction");
	this->ResetSW.Read(exINI, pID, "SpyEffect.ResetSuperweapons");
	this->ResetRadar.Read(exINI, pID, "SpyEffect.ResetRadar");
	this->RevealRadar.Read(exINI, pID, "SpyEffect.RevealRadar");
	this->RevealRadarPersist.Read(exINI, pID, "SpyEffect.KeepRadar");
	this->GainVeterancy.Read(exINI, pID, "SpyEffect.UnitVeterancy");
	this->StolenTechIndex.Read(exINI, pID, "SpyEffect.StolenTechIndex");
	this->PowerOutageDuration.Read(exINI, pID, "SpyEffect.PowerOutageDuration");
	this->StolenMoneyAmount.Read(exINI, pID, "SpyEffect.StolenMoneyAmount");
	this->StolenMoneyPercentage.Read(exINI, pID, "SpyEffect.StolenMoneyPercentage");
	this->UnReverseEngineer.Read(exINI, pID, "SpyEffect.UndoReverseEngineer");

	if(this->StolenTechIndex >= 32) {
		Debug::DevLog(Debug::Warning, "BuildingType %s has a SpyEffect.StolenTechIndex of %d. The value has to be less than 32.\n", pID, this->StolenTechIndex.Get());
		this->StolenTechIndex = -1;
	}

	// #218 Specific Occupiers
	this->AllowedOccupiers.Read(exINI, pID, "CanBeOccupiedBy");
	if(!this->AllowedOccupiers.empty()) {
		// having a specific occupier list implies that this building is supposed to be occupiable
		pThis->CanBeOccupied = true;
	}

	this->Returnable.Read(exINI, pID, "Returnable");

	this->ReverseEngineersVictims.Read(exINI, pID, "ReverseEngineersVictims");

	this->CloningFacility.Read(exINI, pID, "CloningFacility");
	this->Factory_ExplicitOnly.Read(exINI, pID, "Factory.ExplicitOnly");

	this->GateDownSound.Read(exINI, pID, "GateDownSound");
	this->GateUpSound.Read(exINI, pID, "GateUpSound");

	this->IsPassable.Read(exINI, pID, "IsPassable");

	this->AcademyWhitelist.Read(exINI, pID, "Academy.Types");
	this->AcademyBlacklist.Read(exINI, pID, "Academy.Ignore");
	this->AcademyInfantry.Read(exINI, pID, "Academy.InfantryVeterancy");
	this->AcademyAircraft.Read(exINI, pID, "Academy.AircraftVeterancy");
	this->AcademyVehicle.Read(exINI, pID, "Academy.VehicleVeterancy");
	this->AcademyBuilding.Read(exINI, pID, "Academy.BuildingVeterancy");
	this->Academy.clear();
}

void BuildingTypeExt::ExtData::CompleteInitialization(BuildingTypeClass *pThis) {
	// enforce same foundations for rubble/intact building pairs
	if(this->RubbleDestroyed && !BuildingTypeExt::IsFoundationEqual(this->OwnerObject(), this->RubbleDestroyed)) {
		Debug::FatalErrorAndExit("BuildingType %s and its Rubble.Destroyed %s don't have the same foundation.", this->OwnerObject()->ID, this->RubbleDestroyed->ID);
	}
	if(this->RubbleIntact && !BuildingTypeExt::IsFoundationEqual(this->OwnerObject(), this->RubbleIntact)) {
		Debug::FatalErrorAndExit("BuildingType %s and its Rubble.Intact %s don't have the same foundation.", this->OwnerObject()->ID, this->RubbleIntact->ID);
	}
}

void BuildingTypeExt::UpdateSecretLabOptions(BuildingClass *pThis)
{
	BuildingTypeClass *pType = pThis->Type;
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pType);

	Debug::Log("Secret Lab update for %s\n", pType->get_ID());

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
				case HouseExt::RequirementStatus::Forbidden:
				case HouseExt::RequirementStatus::Incomplete:
					ShouldAdd = true;
					break;
			}
			if(ShouldAdd) {
				Options.AddItem(Option);
			}
		}
	}

	if(Options.Count < 1) {
		Debug::Log("Secret Lab [%s] has no boons applicable to country [%s]!\n",
			pType->ID, Owner->Type->ID);
		return;
	}

	int idx = ScenarioClass::Instance->Random.RandomRanged(0, Options.Count - 1);
	Result = Options[idx];

	Debug::Log("Secret Lab rolled %s for %s\n", Result->ID, pType->ID);
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
		if(pTBldA->Foundation != BuildingTypeExt::CustomFoundation) {
			return true;
		}

		// custom foundation
		auto pDataA = BuildingTypeExt::ExtMap.Find(pTBldA);
		auto pDataB = BuildingTypeExt::ExtMap.Find(pTBldB);
		if(pDataA->CustomWidth == pDataB->CustomWidth) {
			if(pDataA->CustomHeight == pDataB->CustomHeight) {
				// compare unsorted arrays the hard way: O(n²)
				return std::all_of(pDataA->CustomData.begin(), pDataA->CustomData.end(), [&](const CellStruct& cell) -> bool {
					auto it2 = std::find(pDataB->CustomData.begin(), pDataB->CustomData.end(), cell);
					return (it2 != pDataB->CustomData.end());
				});
			}
		}
	}
	return false;
}

//! Updates the set of points used to draw this building foundation on the radar.
/*!
	Implements the same logic as used by the original game. That is, buildings
	appear as scaled and tilted rectangles. Only the foundation width and
	height is used. Empty cells in the foundation are filled anyhow.

	\author AlexB
	\date 2013-04-25
*/
void BuildingTypeExt::ExtData::UpdateFoundationRadarShape() {
	this->FoundationRadarShape.Clear();

	if(this->IsCustom) {
		auto pType = this->OwnerObject();
		auto pRadar = RadarClass::Global();

		int width = pType->GetFoundationWidth();
		int height = pType->GetFoundationHeight(false);

		// transform between cell length and pixels on radar
		auto Transform = [](int length, double factor) -> int {
			double dblLength = length * factor + 0.5;
			double minLength = (length == 1) ? 1.0 : 2.0;

			if(dblLength < minLength) {
				dblLength = minLength;
			}

			return Game::F2I(dblLength);
		};

		// the transformed lengths
		int pixelsX = Transform(width, pRadar->RadarSizeFactor);
		int pixelsY = Transform(height, pRadar->RadarSizeFactor);

		// heigth of the foundation tilted by 45°
		int rows = pixelsX + pixelsY - 1;

		// this draws a rectangle standing on an edge, getting
		// wider for each line drawn. the start and end values
		// are special-cased to not draw the pixels outside the
		// foundation.
		for(int i=0; i<rows; ++i) {
			int start = -i;
			if(i >= pixelsY) {
				start = i - 2 * pixelsY + 2;
			}

			int end = i;
			if(i >= pixelsX) {
				end = 2 * pixelsX - i - 2;
			}

			// fill the line
			for(int j=start; j<=end; ++j) {
				Point2D pixel = {j, i};
				this->FoundationRadarShape.AddItem(pixel);
			}
		}
	}
}

// Short check: Is the building of a linkable kind at all?
bool BuildingTypeExt::ExtData::IsLinkable() {
	return this->Firewall_Is || (this->IsTrench > -1);
}

bool BuildingTypeExt::ExtData::CanBeOccupiedBy(InfantryClass *whom) {
	// if CanBeOccupiedBy isn't empty, we have to check if this soldier is allowed in
	return this->AllowedOccupiers.empty() || this->AllowedOccupiers.Contains(whom->Type);
}

bool BuildingTypeExt::ExtData::IsAcademy() const {
	if(this->Academy.empty()) {
		this->Academy = this->AcademyInfantry > 0.0
			|| this->AcademyAircraft > 0.0
			|| this->AcademyVehicle > 0.0
			|| this->AcademyBuilding > 0.0;
	}

	return this->Academy;
}

// =============================
// load / save

bool Container<BuildingTypeExt>::Load(BuildingTypeClass *pThis, IStream *pStm) {
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	// if there's custom data, assign it
	if(pData->IsCustom && pData->CustomWidth > 0 && pData->CustomHeight > 0) {
		pThis->Foundation = BuildingTypeExt::CustomFoundation;
		pThis->FoundationData = pData->CustomData.data();
		pThis->FoundationOutside = pData->OutlineData.data();
	} else {
		pData->CustomData.clear();
		pData->OutlineData.clear();
	}

	return pData != nullptr;
};

// =============================
// container hooks

DEFINE_HOOK(45E50C, BuildingTypeClass_CTOR, 6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(45E707, BuildingTypeClass_DTOR, 6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(465300, BuildingTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(465010, BuildingTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BuildingTypeExt>::PrepareStream(pItem, pStm);

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

DEFINE_HOOK_AGAIN(464A56, BuildingTypeClass_LoadFromINI, A)
DEFINE_HOOK(464A49, BuildingTypeClass_LoadFromINI, A)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

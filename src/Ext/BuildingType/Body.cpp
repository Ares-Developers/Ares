#include "Body.h"
#include "../TechnoType/Body.h"
#include "../House/Body.h"
#include "../SWType/Body.h"
#include "../../Utilities/TemplateDef.h"

#include <InfantryClass.h>
#include <SuperClass.h>
#include <VocClass.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

std::vector<std::string> BuildingTypeExt::ExtData::trenchKinds;
const CellStruct BuildingTypeExt::FoundationEndMarker = {0x7FFF, 0x7FFF};

// =============================
// member funcs

void BuildingTypeExt::ExtData::Initialize()
{
	this->PrismForwarding.Initialize(this->OwnerObject());

	this->LostEvaEvent = VoxClass::FindIndex("EVA_TechBuildingLost");
}

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	char* pArtID = pThis->ImageFile;
	char* pID = pThis->ID;

	INI_EX exINI(pINI);

	this->PrismForwarding.LoadFromINIFile(pThis, pINI);

	if(pThis->UnitRepair && pThis->Factory == AbstractType::AircraftType) {
		Debug::FatalErrorAndExit(
			"BuildingType [%s] has both UnitRepair=yes and Factory=AircraftType.\n"
			"This combination causes Internal Errors and other unwanted behaviour.", pID);
	}

	this->Firewall_Is.Read(exINI, pID, "Firestorm.Wall");

	CCINIClass* pArtINI = CCINIClass::INI_Art;

	// kept for backwards-compatibility with Ares <= 0.9
	INI_EX exArt(pArtINI);
	this->Solid_Height.Read(exArt, pArtID, "SolidHeight");
	
	// relocated the solid tag from artmd to rulesmd
	this->Solid_Height.Read(exINI, pID, "SolidHeight");
	this->Solid_Level.Read(exINI, pID, "SolidLevel");

	if(this->IsCustom) {
		//Reset
		pThis->Foundation = BuildingTypeExt::CustomFoundation;
		pThis->FoundationData = this->CustomData.data();
	} else if(pArtINI) {

		char str[0x80];
		str[0] = '\0';

		if(pArtINI->ReadString(pArtID, "Foundation", "", str) && !_strcmpi(str, "Custom")) {
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

			using Iter = std::vector<CellStruct>::iterator;

			auto ParsePoint = [](Iter &cell, const char* str) -> void {
				int x = 0, y = 0;
				switch(sscanf_s(str, "%d,%d", &x, &y)) {
				case 0:
					x = 0;
					// fallthrough
				case 1:
					y = 0;
				}
				*cell++ = CellStruct{static_cast<short>(x), static_cast<short>(y)};
			};

			auto CellLess = [](const CellStruct& lhs, const CellStruct& rhs) {
				if(lhs.Y != rhs.Y) {
					return lhs.Y < rhs.Y;
				}
				return lhs.X < lhs.X;
			};

			//Load FoundationData
			auto itData = this->CustomData.begin();
			char key[0x20];

			for(int i = 0; i < this->CustomWidth * this->CustomHeight; ++i) {
				_snprintf_s(key, _TRUNCATE, "Foundation.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str)) {
					ParsePoint(itData, str);
				} else {
					break;
				}
			}

			//Sort, remove dupes, add end marker
			std::sort(this->CustomData.begin(), itData, CellLess);
			itData = std::unique(this->CustomData.begin(), itData);
			*itData = FoundationEndMarker;
			this->CustomData.erase(itData + 1, this->CustomData.end());

			auto itOutline = this->OutlineData.begin();
			for(int i = 0; i < this->OutlineLength; ++i) {
				_snprintf_s(key, _TRUNCATE, "FoundationOutline.%d", i);
				if(pArtINI->ReadString(pArtID, key, "", str)) {
					ParsePoint(itOutline, str);
				} else {
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					*itOutline++ = FoundationEndMarker;
				}
			}

			//Set end vector
			*itOutline = FoundationEndMarker;
		}
	}

	this->Secret_Boons.Read(exINI, pID, "SecretLab.PossibleBoons");
	this->Secret_RecalcOnCapture.Read(exINI, pID, "SecretLab.GenerateOnCapture");

	// added on 11.11.09 for #221 and children (Trenches)
	this->UCPassThrough.Read(exINI, pID, "UC.PassThrough");
	this->UCFatalRate.Read(exINI, pID, "UC.FatalRate");
	this->UCDamageMultiplier.Read(exINI, pID, "UC.DamageMultiplier");
	this->BunkerRaidable.Read(exINI, pID, "Bunker.Raidable");
	if(pINI->ReadString(pID, "IsTrench", "", Ares::readBuffer)) {
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

	this->LightningRod_Modifier.Read(exINI, pID, "LightningRod.Modifier");

//	this->LegacyRadarEffect = pINI->ReadBool(pID, "SpyEffect.LegacyRadar", this->LegacyRadarEffect);
//	this->DisplayProduction = pINI->ReadBool(pID, "SpyEffect.DisplayProduction", this->DisplayProduction);

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
		Debug::Log(Debug::Severity::Error, "BuildingType %s has a SpyEffect.StolenTechIndex of %d. The value has to be less than 32.\n", pID, this->StolenTechIndex.Get());
		Debug::RegisterParserError();
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

	this->SuperWeapons.Read(exINI, pID, "SuperWeapons");

	this->LostEvaEvent.Read(exINI, pID, "LostEvaEvent");
	this->MessageCapture.Read(exINI, pID, "Message.Capture");
	this->MessageLost.Read(exINI, pID, "Message.Lost");

	this->DegradeAmount.Read(exINI, pID, "Degrade.Amount");
	this->DegradePercentage.Read(exINI, pID, "Degrade.Percentage");

	this->ImmuneToSaboteurs.Read(exINI, pID, "ImmuneToSaboteurs");

	this->AIBuildCounts.Read(exINI, pID, "AIBuildCounts");
	this->AIExtraCounts.Read(exINI, pID, "AIExtraCounts");

	this->BuildupTime.Read(exINI, pID, "BuildupTime");
}

void BuildingTypeExt::ExtData::CompleteInitialization() {
	auto const pThis = this->OwnerObject();

	// enforce same foundations for rubble/intact building pairs
	if(this->RubbleDestroyed &&
		!BuildingTypeExt::IsFoundationEqual(pThis, this->RubbleDestroyed))
	{
		Debug::FatalErrorAndExit(
			"BuildingType %s and its %s %s don't have the same foundation.",
			pThis->ID, "Rubble.Destroyed", this->RubbleDestroyed->ID);
	}
	if(this->RubbleIntact &&
		!BuildingTypeExt::IsFoundationEqual(pThis, this->RubbleIntact))
	{
		Debug::FatalErrorAndExit(
			"BuildingType %s and its %s %s don't have the same foundation.",
			pThis->ID, "Rubble.Intact", this->RubbleIntact->ID);
	}
}

// returns whether two buildings have the same foundation
bool BuildingTypeExt::IsFoundationEqual(
	BuildingTypeClass const* const pType1,
	BuildingTypeClass const* const pType2)
{
	// both types must be set and must have same foundation id
	if(!pType1 || !pType2 || pType1->Foundation != pType2->Foundation) {
		return false;
	}

	// non-custom foundations need no special handling
	if(pType1->Foundation != BuildingTypeExt::CustomFoundation) {
		return true;
	}

	// custom foundation
	auto const pExt1 = BuildingTypeExt::ExtMap.Find(pType1);
	auto const pExt2 = BuildingTypeExt::ExtMap.Find(pType2);
	const auto& data1 = pExt1->CustomData;
	const auto& data2 = pExt2->CustomData;

	// this works for any two foundations. it's linear with sorted ones
	return pExt1->CustomWidth == pExt2->CustomWidth
		&& pExt1->CustomHeight == pExt2->CustomHeight
		&& std::is_permutation(
			data1.begin(), data1.end(), data2.begin(), data2.end());
}

bool BuildingTypeExt::IsSabotagable(BuildingTypeClass const* const pType)
{
	auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
	auto const civ_occupiable = pType->CanBeOccupied && pType->TechLevel == -1;
	auto const default_sabotabable = pType->CanC4 && !civ_occupiable;

	return !pExt->ImmuneToSaboteurs.Get(!default_sabotabable);
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

size_t BuildingTypeExt::ExtData::GetSuperWeaponCount() const {
	return 2 + this->SuperWeapons.size();
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const size_t index) const {
	const auto pThis = this->OwnerObject();

	if(index < 2) {
		return !index ? pThis->SuperWeapon : pThis->SuperWeapon2;
	} else if(index - 2 < this->SuperWeapons.size()) {
		return this->SuperWeapons[index - 2];
	}

	return -1;
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const size_t index, HouseClass* pHouse) const {
	auto idxSW = this->GetSuperWeaponIndex(index);

	if(auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW)) {
		auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		if(!pExt->IsAvailable(pHouse)) {
			return -1;
		}
	}

	return idxSW;
}

// =============================
// load / save

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Solid_Height)
		.Process(this->Solid_Level)
		.Process(this->IsCustom)
		.Process(this->CustomWidth)
		.Process(this->CustomHeight)
		.Process(this->OutlineLength)
		.Process(this->CustomData)
		.Process(this->OutlineData)
		.Process(this->FoundationRadarShape)
		.Process(this->Secret_Boons)
		.Process(this->Secret_RecalcOnCapture)
		.Process(this->Firewall_Is)
		.Process(this->IsPassable)
		.Process(this->LightningRod_Modifier)
		.Process(this->UCPassThrough)
		.Process(this->UCFatalRate)
		.Process(this->UCDamageMultiplier)
		.Process(this->BunkerRaidable)
		.Process(this->IsTrench)
		.Process(this->RubbleIntact)
		.Process(this->RubbleDestroyed)
		.Process(this->RubbleDestroyedAnim)
		.Process(this->RubbleIntactAnim)
		.Process(this->RubbleDestroyedOwner)
		.Process(this->RubbleIntactOwner)
		.Process(this->RubbleDestroyedStrength)
		.Process(this->RubbleIntactStrength)
		.Process(this->RubbleDestroyedRemove)
		.Process(this->RubbleIntactRemove)
		.Process(this->InfiltrateCustom)
		.Process(this->RevealProduction)
		.Process(this->ResetSW)
		.Process(this->ResetRadar)
		.Process(this->RevealRadar)
		.Process(this->RevealRadarPersist)
		.Process(this->GainVeterancy)
		.Process(this->UnReverseEngineer)
		.Process(this->StolenTechIndex)
		.Process(this->StolenMoneyAmount)
		.Process(this->StolenMoneyPercentage)
		.Process(this->PowerOutageDuration)
		.Process(this->AllowedOccupiers)
		.Process(this->Returnable)
		.Process(this->PrismForwarding)
		.Process(this->ReverseEngineersVictims)
		.Process(this->CloningFacility)
		.Process(this->Factory_ExplicitOnly)
		.Process(this->GateDownSound)
		.Process(this->GateUpSound)
		.Process(this->Academy)
		.Process(this->AcademyWhitelist)
		.Process(this->AcademyBlacklist)
		.Process(this->AcademyInfantry)
		.Process(this->AcademyAircraft)
		.Process(this->AcademyVehicle)
		.Process(this->AcademyBuilding)
		.Process(this->SuperWeapons)
		.Process(this->LostEvaEvent)
		.Process(this->MessageCapture)
		.Process(this->MessageLost)
		.Process(this->DegradeAmount)
		.Process(this->DegradePercentage)
		.Process(this->ImmuneToSaboteurs)
		.Process(this->AIBuildCounts)
		.Process(this->AIExtraCounts)
		.Process(this->BuildupTime);
}

void BuildingTypeExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm) {
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

bool BuildingTypeExt::LoadGlobals(AresStreamReader& Stm) {
	Stm.Process(ExtData::trenchKinds);

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(AresStreamWriter& Stm) {
	Stm.Process(ExtData::trenchKinds);

	return Stm.Success();
}

// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") {
}

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

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
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

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

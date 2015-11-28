#include "ParaDrop.h"
#include "../../Ares.h"
#include "../../Ext/HouseType/Body.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

#include <AircraftClass.h>
#include <HouseClass.h>
#include <SideClass.h>

#include <algorithm>

bool SW_ParaDrop::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ParaDrop) || (type == SuperWeaponType::AmerParaDrop);
}

void SW_ParaDrop::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// default for american paradrop
	if(pSW->Type == SuperWeaponType::AmerParaDrop) {
		// the American paradrop will be the same for every country,
		// thus we use the SW's default here.
		pData->ParaDropPlanes.push_back(std::make_unique<ParadropPlane>());

		auto const& pPlane = pData->ParaDropPlanes.back();
		pData->ParaDrop[nullptr].push_back(pPlane.get());

		auto const& Inf = RulesClass::Instance->AmerParaDropInf;
		pPlane->Types.insert(pPlane->Types.end(), Inf.begin(), Inf.end());

		auto const& Num = RulesClass::Instance->AmerParaDropNum;
		pPlane->Num.insert(pPlane->Num.end(), Num.begin(), Num.end());
	}

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndex("EVA_ReinforcementsReady");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::ParaDrop);
}

void SW_ParaDrop::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	char base[0x40];

	auto CreateParaDropBase = [](char* pID, auto& Buffer) {
		// put a string like "Paradrop.Americans" into the buffer
		if(pID && strlen(pID)) {
			_snprintf_s(Buffer, _TRUNCATE, "ParaDrop.%s", pID);
		} else {
			AresCRT::strCopy(Buffer, "ParaDrop");
		}
	};

	auto ParseParaDrop = [section, &exINI](char* pID, int Plane) -> std::unique_ptr<ParadropPlane> {
		auto pPlane = std::make_unique<ParadropPlane>();

		// create the plane part of this request. this will be
		// an empty string for the first plane for this is the default.
		char plane[0x10] = "";
		if(Plane) {
			_snprintf_s(plane, _TRUNCATE, ".Plane%d", Plane + 1);
		}

		// construct the full tag name base
		char base[0x40];
		_snprintf_s(base, _TRUNCATE, "%s%s", pID, plane);

		// parse the plane contents
		char key[0x40];
		_snprintf_s(key, _TRUNCATE, "%s.Aircraft", base);
		pPlane->Aircraft.Read(exINI, section, key);

		// a list of UnitTypes and InfantryTypes
		_snprintf_s(key, _TRUNCATE, "%s.Types", base);
		pPlane->Types.Read(exINI, section, key);

		// remove all types that cannot paradrop
		Helpers::Alex::remove_non_paradroppables(pPlane->Types, section, key);

		// don't parse nums if there are no types
		if(!pPlane->Aircraft && pPlane->Types.empty()) {
			return nullptr;
		}

		// the number how many times each item is created
		_snprintf_s(key, _TRUNCATE, "%s.Num", base);
		pPlane->Num.Read(exINI, section, key);

		return pPlane;
	};

	auto GetParadropPlane = [=, &base](char const* pID, size_t defaultCount, AbstractTypeClass* pKey) {
		auto const existed = pData->ParaDrop.contains(pKey);
		auto& ParaDrop = pData->ParaDrop[pKey];

		auto const lastCount = existed ? ParaDrop.size() : defaultCount;

		// get the number of planes for this house or side
		char key[0x40];
		_snprintf_s(key, _TRUNCATE, "%s.Count", pID);
		auto const count = pINI->ReadInteger(section, key, lastCount);

		// parse every plane
		ParaDrop.resize(static_cast<size_t>(count), nullptr);
		for(int i = 0; i < count; ++i) {
			if(auto pPlane = ParseParaDrop(base, i)) {
				ParaDrop[i] = pPlane.get();
				pData->ParaDropPlanes.push_back(std::move(pPlane));
			}
		}
	};

	// now load the paradrops
	// 0: default
	// 1 to n: n sides
	// n+1 to n+m+1: m countries

	// default
	CreateParaDropBase(nullptr, base);
	GetParadropPlane(base, 1, nullptr);

	// put all sides into the hash table
	for(auto const& pSide : *SideClass::Array) {
		CreateParaDropBase(pSide->ID, base);
		GetParadropPlane(base, pData->ParaDrop[nullptr].size(), pSide);
	}

	// put all countries into the hash table
	for(auto const& pTHouse : *HouseTypeClass::Array) {
		CreateParaDropBase(pTHouse->ID, base);
		GetParadropPlane(base, pData->ParaDrop[SideClass::Array->Items[pTHouse->SideIndex]].size(), pTHouse);
	}
}

bool SW_ParaDrop::Activate(
	SuperClass* const pThis, const CellStruct &Coords, bool const IsPlayer)
{
	if(pThis->IsCharged) {
		auto pTarget = MapClass::Instance->TryGetCellAt(Coords);

		// find the nearest cell the paradrop troopers can land on
		if(pTarget && pTarget->Tile_Is_Water()) {
			auto const cell = MapClass::Instance->Pathfinding_Find(
				Coords, SpeedType::Foot, -1, MovementZone::Normal, false, 1, 1,
				false, false, false, true, CellStruct::Empty, false, false);

			if(cell != CellStruct::Empty) {
				if(auto const pTemp = MapClass::Instance->TryGetCellAt(cell)) {
					if(!pTemp->Tile_Is_Water()) {
						pTarget = pTemp;
					}
				}
			}
		}

		// all set. send in the planes.
		return this->SendParadrop(pThis, pTarget);
	}

	return false;
}

// Sends the paradrop planes for the given country to the cell specified.
/*
	Every house can have several planes defined. If a plane is not defined by a
	house, this falls back to the side's planes defined for this SW. If that
	fails also it falls back to this SW's default paradrop. If that also fails,
	the paradrop defined by the house is used.

	\param pHouse The owner of this super weapon.
	\param pCell The paradrop target cell.

	\author AlexB
	\date 2010-07-19
*/
bool SW_ParaDrop::SendParadrop(SuperClass* pThis, CellClass* pCell)
{
	// sanity
	if(!pThis || !pCell) {
		return false;
	}

	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);
	auto const pHouse = pThis->Owner;

	// these are fallback values if the SW doesn't define them
	AircraftTypeClass* pFallbackPlane = nullptr;
	Iterator<TechnoTypeClass*> FallbackTypes;
	Iterator<int> FallbackNum;

	if(auto const pExt = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
		pExt->GetParadropContent(FallbackTypes, FallbackNum);
		pFallbackPlane = pExt->GetParadropPlane();
	}

	// use paradrop lists from house, side and default
	std::vector<ParadropPlane*>* drops[3];
	drops[0] = pData->ParaDrop.find(pHouse->Type);
	drops[1] = pData->ParaDrop.find(SideClass::Array->Items[pHouse->Type->SideIndex]);
	drops[2] = pData->ParaDrop.find(nullptr);

	// how many planes shall we launch?
	int count = 0;
	for(auto const& planes : drops) {
		if(planes) {
			count = planes->size();
			break;
		}
	}

	// assemble each plane and its contents
	for(int i = 0; i < count; ++i) { // i = index of plane
		Iterator<TechnoTypeClass*> ParaDropTypes;
		Iterator<int> ParaDropNum;
		AircraftTypeClass* pParaDropPlane = nullptr;

		// try the planes in order of precedence:
		// * country, explicit plane
		// * side, explicit plane
		// * default, explict plane
		// * country, default plane
		// * side, default plane
		// * default, default plane
		// * fill gaps with data from house/side/rules
		for(int j = 1; j >= 0; --j) { // factor 1 or 0: "plane * j" => "plane" or "0" (default)
			for(auto const& planes : drops) { // get the country/side-specific plane list

				// only do something if there is data missing
				if(!(ParaDropTypes && ParaDropNum && pParaDropPlane)) {
					auto const index = static_cast<size_t>(i * j);

					if(!planes || planes->size() <= index) {
						continue;
					}

					// get the plane at specified index
					if(auto const pPlane = (*planes)[index]) {

						// get the contents, if not already set
						if(!ParaDropTypes || !ParaDropNum) {
							if(!pPlane->Types.empty() && !pPlane->Num.empty()) {
								ParaDropTypes = pPlane->Types;
								ParaDropNum = pPlane->Num;
							}
						}

						// get the airplane, if it isn't set already
						if(!pParaDropPlane) {
							pParaDropPlane = pPlane->Aircraft;
						}
					}
				}
			}
		}

		// fallback for types and nums
		if(!ParaDropTypes || !ParaDropNum) {
			ParaDropTypes = FallbackTypes;
			ParaDropNum = FallbackNum;
		}

		// house fallback for the plane
		if(!pParaDropPlane) {
			pParaDropPlane = pFallbackPlane;
		}

		// finally, send the plane
		if(ParaDropTypes && ParaDropNum && pParaDropPlane) {
			SendPDPlane(pHouse, pCell, pParaDropPlane, ParaDropTypes, ParaDropNum);
		}
	}

	return true;
}

//A new SendPDPlane function
//Allows vehicles, sends one single plane for all types
void SW_ParaDrop::SendPDPlane(HouseClass* pOwner, CellClass* pTarget, AircraftTypeClass* pPlaneType,
	Iterator<TechnoTypeClass*> const Types, Iterator<int> const Nums)
{
	if(Nums.size() != Types.size() || !Nums.size()
		|| !pOwner || !pPlaneType || !pTarget)
	{
		return;
	}

	++Unsorted::IKnowWhatImDoing;
	auto const pPlane = static_cast<AircraftClass*>(
		pPlaneType->CreateObject(pOwner));
	--Unsorted::IKnowWhatImDoing;

	pPlane->Spawned = true;

	//Get edge (direction for plane to come from)
	auto edge = pOwner->StartingEdge;
	if(edge < Edge::North || edge > Edge::West) {
		edge = pOwner->Edge;
		if(edge < Edge::North || edge > Edge::West) {
			edge = Edge::North;
		}
	}

	// seems to retrieve a random cell struct at a given edge
	auto const spawn_cell = MapClass::Instance->PickCellOnEdge(
		edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
		MovementZone::Normal);

	pPlane->QueueMission(Mission::ParadropApproach, false);

	if(pTarget) {
		pPlane->SetTarget(pTarget);
	}

	auto const spawn_crd = CellClass::Cell2Coord(spawn_cell);

	++Unsorted::IKnowWhatImDoing;
	auto const bSpawned = pPlane->Put(spawn_crd, Direction::North);
	--Unsorted::IKnowWhatImDoing;

	if(!bSpawned) {
		GameDelete(pPlane);
		return;
	}

	for(auto i = 0u; i < Types.size(); ++i) {
		auto const pType = Types[i];

		//only allow infantry and vehicles
		auto const abs = pType->WhatAmI();
		if(abs == AbstractType::UnitType || abs == AbstractType::InfantryType) {
			for(int k = 0; k < Nums[i]; ++k) {
				auto const pNew = pType->CreateObject(pOwner);
				pNew->Remove();
				pPlane->Passengers.AddPassenger(static_cast<FootClass*>(pNew));
			}
		}
	}

	pPlane->HasPassengers = true;
	pPlane->NextMission();
}

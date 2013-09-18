#include "ParaDrop.h"
#include "../../Ares.h"
#include "../../Ext/HouseType/Body.h"

#include <SideClass.h>

bool SW_ParaDrop::HandlesType(int type)
{
	return (type == SuperWeaponType::ParaDrop) || (type == SuperWeaponType::AmerParaDrop);
}

void SW_ParaDrop::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// default for american paradrop
	if(pSW->Type == SuperWeaponType::AmerParaDrop) {
		// the American paradrop will be the same for every country,
		// thus we use the SW's default here.
		ParadropPlane* pPlane = new ParadropPlane();
		pData->ParaDropPlanes.AddItem(pPlane);
		pData->ParaDrop[NULL].AddItem(pPlane);

		for(int i = 0; i < RulesClass::Instance->AmerParaDropInf.Count; ++i) {
			pPlane->pTypes.AddItem((RulesClass::Instance->AmerParaDropInf.GetItem(i)));
		}

		for(int i = 0; i < RulesClass::Instance->AmerParaDropNum.Count; ++i) {
			pPlane->pNum.AddItem(RulesClass::Instance->AmerParaDropNum.GetItem(i));
		}
	}

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndex("EVA_ReinforcementsReady");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::ParaDrop];
}

void SW_ParaDrop::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	char base[0x40];

	auto CreateParaDropBase = [](char* pID, char* pBuffer) {
		// put a string like "Paradrop.Americans" into the buffer
		if(pBuffer) {
			AresCRT::strCopy(pBuffer, "ParaDrop", 9);
			if(pID && strlen(pID)) {
				AresCRT::strCopy(&pBuffer[8], ".", 2);
				AresCRT::strCopy(&pBuffer[9], pID, 0x18);
			}
		}
	};

	auto ParseParaDrop = [&](char* pID, int Plane) -> ParadropPlane* {
		ParadropPlane* pPlane = NULL;

		// create the plane part of this request. this will be
		// an empty string for the first plane for this is the default.
		char plane[0x10] = "";
		if(Plane) {
			AresCRT::strCopy(plane, ".Plane", 0x10);
			_itoa_s(Plane + 1, &plane[6], 10, 10);
		}
		
		// construct the full tag name base
		char base[0x40], key[0x40];
		_snprintf_s(base, 0x3F, "%s%s", pID, plane);

		// parse the plane contents
		_snprintf_s(key, 0x3F, "%s.Aircraft", base);
		if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			if(AircraftTypeClass* pTAircraft = AircraftTypeClass::Find(Ares::readBuffer)) {
				pPlane = new ParadropPlane();
				pPlane->pAircraft = pTAircraft;
			} else {
				Debug::INIParseFailed(section, key, Ares::readBuffer);
			}
		}

		// a list of UnitTypes and InfantryTypes
		_snprintf_s(key, 0x3F, "%s.Types", base);
		if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			// create new plane if there is none yet
			if(!pPlane) {
				pPlane = new ParadropPlane();
			}

			// parse the types
			pPlane->pTypes.Clear();

			char* context = nullptr;
			for(char* p = strtok_s(Ares::readBuffer, Ares::readDelims, &context); p && *p; p = strtok_s(nullptr, Ares::readDelims, &context)) {
				TechnoTypeClass* pTT = UnitTypeClass::Find(p);

				if(!pTT) {
					pTT = InfantryTypeClass::Find(p);
				}

				if(pTT) {
					pPlane->pTypes.AddItem(pTT);
				} else {
					Debug::INIParseFailed(section, key, p);
				}
			}
		}

		// don't parse nums if there are no types
		if(!pPlane || !pPlane->pTypes.Count) {
			return pPlane;
		}

		// the number how many times each item is created
		_snprintf_s(key, 0x3F, "%s.Num", base);
		if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			pPlane->pNum.Clear();

			char* context = nullptr;
			for(char* p = strtok_s(Ares::readBuffer, Ares::readDelims, &context); p && *p; p = strtok_s(nullptr, Ares::readDelims, &context)) {
				pPlane->pNum.AddItem(atoi(p));
			}
		}

		return pPlane;
	};

	auto GetParadropPlane = [&](char *pID, int defCount, DynamicVectorClass<ParadropPlane*>* ret) {
		// get the number of planes for this house or side
		char key[0x40];
		_snprintf_s(key, 0x3F, "%s.Count", pID);
		int count = pINI->ReadInteger(section, key, defCount);

		// parse every plane
		ret->SetCapacity(count, NULL);
		for(int i=0; i<count; ++i) {
			if(i>=ret->Count) {
				ret->AddItem(NULL);
			}
			
			ParadropPlane* pPlane = ParseParaDrop(base, i);
			if(pPlane) {
				pData->ParaDropPlanes.AddItem(pPlane);
				ret->Items[i] = pPlane;
			}
		}
	};

	// now load the paradrops
	// 0: default
	// 1 to n: n sides
	// n+1 to n+m+1: m countries

	// default
	CreateParaDropBase(NULL, base);
	GetParadropPlane(base, 1, &pData->ParaDrop[NULL]);

	// put all sides into the hash table
	for(int i=0; i<SideClass::Array->Count; ++i) {
		SideClass *pSide = SideClass::Array->GetItem(i);
		CreateParaDropBase(pSide->ID, base);
		GetParadropPlane(base, pData->ParaDrop[NULL].Count, &pData->ParaDrop[pSide]);
	}

	// put all countries into the hash table
	for(int i=0; i<HouseTypeClass::Array->Count; ++i) {
		HouseTypeClass *pTHouse = HouseTypeClass::Array->GetItem(i);
		CreateParaDropBase(pTHouse->ID, base);
		GetParadropPlane(base, pData->ParaDrop[SideClass::Array->GetItem(pTHouse->SideIndex)].Count, &pData->ParaDrop[pTHouse]);
	}
}

bool SW_ParaDrop::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	if(pThis->IsCharged) {
		CellClass *pTarget = MapClass::Instance->GetCellAt(pCoords);

		// find the nearest cell the paradrop troopers can land on
		if(pTarget != MapClass::InvalidCell()) {
			if(pTarget->Tile_Is_Water()) {
				int a2 = 0;
				int a14 = 0;
				CellStruct *nearest = MapClass::Instance->Pathfinding_Find(&a2, pCoords, 0, -1, 0, 0, 1, 1, 0, 0, 0, 1, &a14, 0, 0);
				if(*nearest != SuperClass::DefaultCoords) {
					if(CellClass *pTemp = MapClass::Instance->GetCellAt(nearest)) {
						if(pTemp != MapClass::InvalidCell()) {
							if(!pTemp->Tile_Is_Water()) {
								pTarget = pTemp;
							}
						}
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
bool SW_ParaDrop::SendParadrop(SuperClass* pThis, CellClass* pCell) {
	// sanity
	if(!pThis || !pCell) {
		return false;
	}

	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);
	HouseClass *pHouse = pThis->Owner;

	// these are fallback values if the SW doesn't define them
	AircraftTypeClass* pFallbackPlane = NULL;
	TypeList<TechnoTypeClass*> *pFallbackTypes = NULL;
	TypeList<int> *pFallbackNum = NULL;

	// get the paradrop list without creating a new value
	auto GetParadropPlanes = [pData](AbstractTypeClass* pKey) -> DynamicVectorClass<ParadropPlane*>* {
		if(pData->ParaDrop.find(pKey) == pData->ParaDrop.end()) {
			return NULL;
		}
		return &pData->ParaDrop[pKey];
	};

	// use paradrop lists from house, side and default
	DynamicVectorClass<ParadropPlane*>* drops[3];
	drops[0] = GetParadropPlanes(pHouse->Type);
	drops[1] = GetParadropPlanes(SideClass::Array->GetItem(pHouse->Type->SideIndex));
	drops[2] = GetParadropPlanes(NULL);

	// how many planes shall we launch?
	int count = 0;
	for(int i=0; i<3; ++i) {
		if(drops[i]) {
			count = drops[i]->Count;
			break;
		}
	}

	// assemble each plane and its contents
	for(int i=0; i<count; ++i) { // i = index of plane
		TypeList<TechnoTypeClass*> *pParaDrop = NULL;
		TypeList<int> *pParaDropNum = NULL;
		AircraftTypeClass* pParaDropPlane = NULL;

		// try the planes in order of precedence:
		// * country, explicit plane
		// * side, explicit plane
		// * default, explict plane
		// * country, default plane
		// * side, default plane
		// * default, default plane
		// * fill gaps with data from house/side/rules
		for(int j=1; j>=0; --j) { // factor 1 or 0: "plane * j" => "plane" or "0" (default)
			for(int k=0; k<3; ++k) { // index in the "drops" array

				// only do something if there is data missing
				if(!(pParaDrop && pParaDropNum && pParaDropPlane)) {
					// get the country/side-specific plane list
					DynamicVectorClass<ParadropPlane*> *planes = drops[k];
					if(!planes) {
						continue;
					}

					// get the plane at specified index
					int index = i * j;
					if(planes->ValidIndex(index)) {
						if(ParadropPlane* pPlane = planes->GetItem(index)) {

							// get the contents, if not already set
							if(!pParaDrop || !pParaDropNum) {
								if((pPlane->pTypes.Count != 0) && (pPlane->pNum.Count != 0)) {
									pParaDrop = &pPlane->pTypes;
									pParaDropNum = &pPlane->pNum;
								}
							}

							// get the airplane, if it isn't set already
							if(!pParaDropPlane) {
								if(AircraftTypeClass* pTAircraft = pPlane->pAircraft) {
									pParaDropPlane = pTAircraft;
								}
							}
						}
					}
				}
			}
		}

		// fallback for types and nums
		if(!pParaDrop || !pParaDropNum) {
			if(!pFallbackTypes || !pFallbackNum) {
				if(HouseTypeExt::ExtData *pExt = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
					pExt->GetParadropContent(&pFallbackTypes, &pFallbackNum);
				}
			}

			pParaDrop = pFallbackTypes;
			pParaDropNum = pFallbackNum;
		}

		// house fallback for the plane
		if(!pParaDropPlane) {
			if(!pFallbackPlane) {
				if(HouseTypeExt::ExtData *pExt = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
					pFallbackPlane = pExt->GetParadropPlane();
				}
			}

			pParaDropPlane = pFallbackPlane;
		}

		// finally, send the plane
		if(pParaDrop && pParaDropNum && pParaDropPlane) {
			Ares::SendPDPlane(
				pHouse,
				pCell,
				pParaDropPlane,
				pParaDrop,
				pParaDropNum);
		}
	}

	return true;
}
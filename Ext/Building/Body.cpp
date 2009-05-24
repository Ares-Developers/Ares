#include "Body.h"
#include "..\BuildingType\Body.h"
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <CellSpread.h>

// =============================
// member functions

void BuildingExt::ExtendFirewall(BuildingClass *pThis, CellStruct Center, HouseClass *Owner) {
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(!pData->Firewall_Is) {
		return;
	}
	int range = pThis->Type->GuardRange / 256;
	CoordStruct XYZ;
	for(int direction = 0; direction <= 7; direction += 2) {
		Debug::Log("Scanning direction %d\n", direction);
		CellStruct offset = CellSpread::GetNeighbourOffset(direction);
		Debug::Log("Offset is %d,%d\n", offset.X, offset.Y);
		int fillRange = 0;
		for(int curRange = 1; curRange <= range; ++curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			Debug::Log("Range %d coords are %d,%d\n", curRange, CurrentPos.X, CurrentPos.Y);
			if(MapClass::Global()->CellExists(&CurrentPos)) {
				CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos);
				Debug::Log("Cell Exists!\n");
				if(BuildingClass *OtherEnd = cell->GetBuilding()) {
					Debug::Log("Cell Is Occupied...\n");
					if(OtherEnd->Owner != Owner || OtherEnd->Type != pThis->Type) {
						fillRange = 0; // can't link
						Debug::Log(" by someone else - cannot proceed\n");
					} else {
						fillRange = curRange - 1;
					}
					break;
				}
			} else {
				Debug::Log("Cell doesn't exist?!\n");
				break;
			}
			Debug::Log("\nNext?\n");
		}

		Debug::Log("So the fillRange is %d\n", fillRange);
		++Unsorted::SomeMutex;
		for(int curRange = fillRange; curRange > 0; --curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			Debug::Log("Placement loop; Range %d coords are %d,%d\n", curRange, CurrentPos.X, CurrentPos.Y);
			if(CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos)) {
				if(BuildingClass *dummy = reinterpret_cast<BuildingClass *>(pThis->Type->CreateObject(Owner))) {
					CellClass::Cell2Coord(&CurrentPos, &XYZ);
					if(!cell->CanThisExistHere(dummy->Type->SpeedType, dummy->Type, Owner) || !dummy->Put(&XYZ, 0)) {
						Debug::Log("Fuck'n Boom!\n");
						delete dummy;
					}
				}
			}
			Debug::Log("\nNext?\n");
		}
		--Unsorted::SomeMutex;
	}
}

DWORD BuildingExt::GetFirewallFlags(BuildingClass *pThis) {
	CellClass *MyCell = MapClass::Global()->GetCellAt(pThis->get_Location());
	DWORD flags = 0;
	for(int direction = 0; direction < 8; direction += 2) {
		CellClass *Neighbour = MyCell->GetNeighbourCell(direction);
		if(BuildingClass *B = Neighbour->GetBuilding()) {
			BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
			if(pTypeData->Firewall_Is && B->Owner == pThis->Owner && !B->InLimbo && B->IsAlive) {
				flags |= 1 << (direction >> 1);
			}
		}
	}
	return flags;
}

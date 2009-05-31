#include "Body.h"
#include "..\BuildingType\Body.h"
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <CellSpread.h>

const DWORD Extension<BuildingClass>::Canary = 0x99999999;
Container<BuildingExt> BuildingExt::ExtMap;

BuildingClass *Container<BuildingExt>::SavingObject = NULL;
IStream *Container<BuildingExt>::SavingStream = NULL;

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
		CellStruct offset = CellSpread::GetNeighbourOffset(direction);
		int fillRange = 0;
		for(int curRange = 1; curRange <= range; ++curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			if(MapClass::Global()->CellExists(&CurrentPos)) {
				CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos);
				if(BuildingClass *OtherEnd = cell->GetBuilding()) {
					if(OtherEnd->Owner != Owner || OtherEnd->Type != pThis->Type) {
						fillRange = 0; // can't link
					} else {
						fillRange = curRange - 1;
					}
					break;
				}
			} else {
				break;
			}
		}

		++Unsorted::SomeMutex;
		for(int curRange = fillRange; curRange > 0; --curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			if(CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos)) {
				if(BuildingClass *dummy = reinterpret_cast<BuildingClass *>(pThis->Type->CreateObject(Owner))) {
					CellClass::Cell2Coord(&CurrentPos, &XYZ);
					if(!cell->CanThisExistHere(dummy->Type->SpeedType, dummy->Type, Owner) || !dummy->Put(&XYZ, 0)) {
						delete dummy;
					}
				}
			}
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

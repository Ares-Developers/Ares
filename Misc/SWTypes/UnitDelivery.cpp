#include "UnitDelivery.h"
#include "../../Ext/Techno/Body.h"

void SW_UnitDelivery::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	if(pINI->ReadString(section, "SW.Deliver", "", Ares::readBuffer, Ares::readLength)) {
		pData->SW_Deliverables.Clear();
		for(char *cur = strtok(Ares::readBuffer, ","); cur && *cur; cur = strtok(NULL, ",")) {
			TechnoTypeClass * Type = InfantryTypeClass::Find(cur);
			if(!Type) {
				Type = UnitTypeClass::Find(cur);
			}
			if(!Type) {
				Type = AircraftTypeClass::Find(cur);
			}
			if(!Type) {
				Type = BuildingTypeClass::Find(cur);
			}
			if(Type) {
				pData->SW_Deliverables.AddItem(Type);
			}
		}
	}
}

bool SW_UnitDelivery::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	this->newStateMachine(150, *pCoords, pThis);

	Unsorted::CurrentSWType = -1;
	return 1;
}

void UnitDeliveryStateMachine::Update() {
	switch(this->TimePassed()) {
	case 1:
		// play anim
		break;
	case 20:
		this->PlaceUnits();
		break;
	case 100:
		// write message
		break;
	};
}

void UnitDeliveryStateMachine::PlaceUnits() {
	int unitIdx = 0;
	int cellIdx = 0;
	SWTypeExt::ExtData *pData = this->FindExtData();

	if(!pData) {
		return;
	}

	while(unitIdx < pData->SW_Deliverables.Count) {
		TechnoTypeClass * Type = pData->SW_Deliverables[unitIdx];
		TechnoClass * Item = reinterpret_cast<TechnoClass *>(Type->CreateObject(this->Super->Owner));
		bool Placed = false;
		do {
			++cellIdx;
			if(cellIdx >= 100) { // 100 cells should be enough for any sane delivery
				unitIdx = 999;
				Item->UnInit();
				break;
			}
			CellStruct tmpCell = CellSpread::GetCell(cellIdx) + this->Coords;
			CoordStruct XYZ;
			if(MapClass::Instance->CellExists(&tmpCell)) {
				CellClass *cell = MapClass::Instance->GetCellAt(&tmpCell);
				cell->GetCoordsWithBridge(&XYZ);
				Placed = Item->Put(&XYZ, (cellIdx & 7));
			}
		} while(!Placed);
		++unitIdx;
	}

}

#include "Body.h"

#include <OverlayClass.h>
#include <OverlayTypeClass.h>

// removing hardcoded references to GAWALL and NAWALL as part of #709
DEFINE_HOOK(440709, BuildingClass_Put, 6)
{
	GET(CellClass *, Cell, EDI);
	int idxOverlay = Cell->OverlayTypeIndex;
	bool Sellable = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	return Sellable ? 0x44071A : 0x440725;
}

DEFINE_HOOK(480534, CellClass_AttachesToNeighbourOverlay, 5)
{
	GET(int, idxOverlay, EAX);
	bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	return Wall ? 0x480549 : 0x480552;
}

DEFINE_HOOK(47C8AB, CellClass_CanThisExistHere_GateOnWall, 6)
{
	GET(CellClass *, pCell, EDI);
	GET(HouseClass *, OverlayOwner, ESI);
	GET(int, idxOverlay, ECX);

	GET_STACK(BuildingTypeClass *, PlacingObject, 0x20);
	GET_STACK(HouseClass *, PlacingOwner, 0x24);

	enum { Adequate = 0x47CA70, Inadequate = 0x47C94F } Status = Inadequate;

	if(PlacingObject) {
		bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;

		if(ContainsWall && PlacingObject->Gate) {
			Status = Adequate;
		}

		if(OverlayTypeClass * ToOverlay = PlacingObject->ToOverlay) {
			if(ToOverlay->ArrayIndex == idxOverlay) {
				if(pCell->Powerup >= 0x10) {
					Status = Adequate;
				}
			}
		}
	}

	if(Status == Inadequate) {
		switch(idxOverlay) {
			case OVERLAY_GASAND:
			case OVERLAY_GAWALL:
				if (PlacingObject == RulesClass::Instance->WallTower ||
						PlacingObject == RulesClass::Instance->GDIGateOne ||
						PlacingObject == RulesClass::Instance->GDIGateTwo) {
						Status = Adequate;
				}
				break;
			case OVERLAY_NAWALL:
				if(PlacingObject == RulesClass::Instance->NodGateOne ||
					PlacingObject == RulesClass::Instance->NodGateTwo) {
						Status = Adequate;
				}
				break;
		}
	}

	if(Status == Adequate) {
		if(PlacingOwner != OverlayOwner) {
			Status = Inadequate;
		}
	}

	return Status;
}

DEFINE_HOOK(44E550, BuildingClass_Mi_Open_GateDown, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	R->ECX(pExt->GateDownSound.Get(RulesClass::Instance->GateDown));
	return 0x44E556;
}

DEFINE_HOOK(44E61E, BuildingClass_Mi_Open_GateUp, 6)
{
	GET(DWORD, offset, ESI);
	auto pThis = reinterpret_cast<BuildingClass*>(offset - 0x9C);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	R->ECX(pExt->GateUpSound.Get(RulesClass::Instance->GateUp));
	return 0x44E624;
}

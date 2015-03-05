#pragma once

#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <Helpers/Other.h>

#include "../../Ext/BuildingType/Body.h"
#include "../../Ext/Building/Body.h"
#include "../../Ext/BulletType/Body.h"
#include "../../Ext/House/Body.h"
#include "../../Ext/TechnoType/Body.h"

class FirestormFinderApplicator : public CellSequenceApplicator {
protected:
	HouseClass *pOwner;
public:
	bool found;
	CellStruct target;

	FirestormFinderApplicator(HouseClass *owner)
		: pOwner(owner), CellSequenceApplicator(), found(false), target(CellStruct::Empty)
	{ }

	void operator() (CellClass* curCell) override {
		if(!found) {
			if(auto const pBuilding = curCell->GetBuilding()) {
				if(BuildingExt::IsActiveFirestormWall(pBuilding, pOwner)) {
					target = curCell->MapCoords;
					found = true;
				}
			}
		}
	}
};

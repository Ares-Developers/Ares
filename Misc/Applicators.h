#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <Helpers\Template.h>

#include "..\..\Ext\BuildingType\Body.h"
#include "..\..\Ext\Building\Body.h"
#include "..\..\Ext\BulletType\Body.h"
#include "..\..\Ext\House\Body.h"
#include "..\..\Ext\TechnoType\Body.h"

class FirestormFinderApplicator : public CellSequenceApplicator {
	protected:
		HouseClass *pOwner;
	public:
		bool found;
		CellStruct target;

		FirestormFinderApplicator(HouseClass *owner)
			: pOwner(owner), CellSequenceApplicator(), found(false)
		{
			target.X = target.Y = 0;
		}
		virtual void operator() (CellClass *curCell) {
			if(!found) {
				if(BuildingClass *B = curCell->GetBuilding()) {
					if(!B->InLimbo && B->Owner != pOwner) {
						BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
						HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);
						if(pTypeData->Firewall_Is && pHouseData->FirewallActive) {
							target = *curCell->get_MapCoords();
							found = 1;
						}
					}
				}
			}
		}
};

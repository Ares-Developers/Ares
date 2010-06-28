#ifndef HELPERS_ALEX_H
#define HELPERS_ALEX_H

#include <set>

class Helpers {
public:
	class Alex {
	public:
		//! Gets the new duration a stackable or absolute effect will last.
		/*!
			The new frames count is calculated the following way:

			If Duration is positive it will inflict damage. If Cap is larger than zero,
			the maximum amount of frames will be defined by Cap. If the current value
			already is larger than that, in will not be reduced. If Cap is zero, then
			the duration can add up infinitely. If Cap is less than zero, duration will
			be set to Duration, if the current value is not higher already.

			If Duration is negative, the effect will be reduced. A negative Cap
			reduces the current value by Duration. A positive or zero Cap will do the
			same, but additionally shorten it to Cap if the result would be higher than
			that. Thus, a Cap of zero removes the current effect altogether.

			\param CurrentValue The Technos current remaining time.
			\param Duration The duration the effect uses.
			\param Cap The maximum Duration this effect can cause.

			\returns The new effect frames count.

			\author AlexB
			\date 2010-04-27
		*/
		static int getCappedDuration(int CurrentValue, int Duration, int Cap) {
			// Usually, the new duration is just added.
			int ProposedDuration = CurrentValue + Duration;

			if (Duration > 0) {
				// Positive damage.
				if (Cap < 0) {
					// Do not stack. Use the maximum value.
					return max(Duration, CurrentValue);
				} else if (Cap > 0) {
					// Cap the duration.
					int cappedValue = min(ProposedDuration, Cap);
					return max(CurrentValue, cappedValue);
				} else {
					// There is no cap. Allow the duration to stack up.
					return ProposedDuration;
				}
			} else {
				// Negative damage.
				return (Cap < 0 ? ProposedDuration : min(ProposedDuration, Cap));
			}
		}

		//! Gets a list of all units in range of a cell spread weapon.
		/*!
			CellSpread is handled as described in
			http://modenc.renegadeprojects.com/CellSpread.

			\param coords The location the projectile detonated.
			\param spread The range to find items in.
			\param includeInAir Include items that are currently InAir.

			\author AlexB
			\date 2010-06-28
		*/
		static DynamicVectorClass<TechnoClass*>* getCellSpreadItems(CoordStruct *coords, float spread, bool includeInAir=false) {
			// set of possibly affected objects. every object can be here only once.
			std::set<TechnoClass*> *set = new std::set<TechnoClass*>();

			// the quick way. only look at stuff residing on the very cells we are affecting.
			CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
			int countCells = CellSpread::NumCells((int)(spread + 0.99));
			for(int i = 0; i < countCells; ++i) {
				CellStruct tmpCell = CellSpread::GetCell(i);
				tmpCell += cellCoords;
				CellClass *c = MapClass::Instance->GetCellAt(&tmpCell);
				for(ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
					if(TechnoClass *Techno = generic_cast<TechnoClass*>(curObj)) {
						set->insert(Techno);
					}
				}
			}

			// flying objects are not included normally
			if(includeInAir) {
				// the not quite so fast way. skip everything not in the air.
				for(int i=0; i<TechnoClass::Array->Count; ++i) {
					TechnoClass *Techno = TechnoClass::Array->GetItem(i);
					if(Techno->IsInAir()) {
						// rough estimation
						if(Techno->Location.DistanceFrom(*coords) <= spread * 256) {
							set->insert(Techno);
						}
					}
				}
			}

			// look closer. the final selection. put all affected items in a vector.
			DynamicVectorClass<TechnoClass*> *ret = new DynamicVectorClass<TechnoClass*>();
			for(std::set<TechnoClass*>::iterator iterator = set->begin(); iterator != set->end(); iterator++) {
				TechnoClass *Techno = *iterator;

				// ignore buildings that are not visible, like ambient light posts
				if(BuildingTypeClass *BT = specific_cast<BuildingTypeClass*>(Techno->GetTechnoType())) {
					if(BT->InvisibleInGame) {
						continue;
					}
				}

				// get distance from impact site
				CoordStruct target;
				Techno->GetCoords(&target);
				double dist = target.DistanceFrom(*coords);

				// reduce the distance for flying aircraft
				if((Techno->WhatAmI() == abs_Aircraft) && Techno->IsInAir()) {
					dist *= 0.5;
				}

				// this is good
				if(dist <= spread * 256) {
					ret->AddItem(Techno);
				}
			}

			// tidy up
			set->clear();
			delete set;

			return ret;
		}
	};
};

#endif
#ifndef HELPERS_ALEX_H
#define HELPERS_ALEX_H

#include <CellSpread.h>

#include <set>
#include <functional>

class Helpers {
public:
	class Alex {
	public:
		//! Comparers to be used with sets.
		template<typename T>
		struct StrictWeakComparer {
			bool operator() (const T& lhs, const T& rhs) const {
				return lhs < rhs;
			}
		};

		template<>
		struct StrictWeakComparer<ObjectClass*> {
			bool operator() (const ObjectClass* lhs, const ObjectClass* rhs) const {
				return lhs->UniqueID < rhs->UniqueID;
			}
		};

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
					return std::max(Duration, CurrentValue);
				} else if (Cap > 0) {
					// Cap the duration.
					int cappedValue = std::min(ProposedDuration, Cap);
					return std::max(CurrentValue, cappedValue);
				} else {
					// There is no cap. Allow the duration to stack up.
					return ProposedDuration;
				}
			} else {
				// Negative damage.
				return (Cap < 0 ? ProposedDuration : std::min(ProposedDuration, Cap));
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
			auto set = new std::set<TechnoClass*, StrictWeakComparer<ObjectClass*> >();

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
					if(Techno->GetHeight() > 0) {
						// rough estimation
						if(Techno->Location.DistanceFrom(*coords) <= spread * 256) {
							set->insert(Techno);
						}
					}
				}
			}

			// look closer. the final selection. put all affected items in a vector.
			DynamicVectorClass<TechnoClass*> *ret = new DynamicVectorClass<TechnoClass*>();
			for(auto iterator = set->begin(); iterator != set->end(); iterator++) {
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

		static int forEach(CellStruct *cell, int width, int height, std::tr1::function<bool (CellClass*)> action) {
			int ret = 0;

			// the coords mark the center of the area
			CellStruct Offset;
			Offset.X = (short)(cell->X - (width / 2));
			Offset.Y = (short)(cell->Y - (height / 2));

			// take a look at each cell in the rectangle
			int cellCount = (width * height);
			for(int i=0; i<cellCount; ++i) {

				// get the specific cell coordinates
				CellStruct Cell;
				Cell.X = (short)(i % width);
				Cell.Y = (short)(i / width);
				Cell += Offset;

				// get this cell and call the action function
				CellClass* pCell = MapClass::Instance->GetCellAt(&Cell);
				if(pCell != MapClass::InvalidCell()) {
					if(action(pCell)) {
						++ret;
					} else {
						break;
					}
				}
			}

			return ret;
		}

		static int forEach(CellStruct *cell, float radius, std::tr1::function<bool (CellClass*)> action) {
			int ret = 0;

			// radius in every direction
			int range = (int)(radius + 0.99) * 2 + 1;

			auto actionIfInRange = [&](CellClass* pCell) -> bool {
				// if it is near enough, do action
				if(cell->DistanceFrom(pCell->MapCoords) <= radius) {
					if(action(pCell)) {
						++ret;
					} else {
						return false;
					}
				}

				return true;
			};

			// get all cells in a square around the target
			forEach(cell, range, range, actionIfInRange);

			return ret;
		}

		static int forEachCellInRange(CellStruct *cell, float widthOrRange, int height, std::tr1::function<bool (CellClass*)> action) {
			if((height > 0) && ((height * widthOrRange) > 0)) {
				// rectangle
				return forEach(cell, (int)widthOrRange, height, action);
			} else if(widthOrRange > 0) {
				// circle
				return forEach(cell, widthOrRange, action);
			}
			return -1;
		}

		static int forEachObjectInRange(CellStruct *cell, float widthOrRange, int height, std::tr1::function<bool (ObjectClass*)> action) {
			int ret = 0;

			int maxDistance = (int)widthOrRange;

			// get target cell cords
			CoordStruct coords;
			CellClass* pTarget = MapClass::Instance->GetCellAt(cell);
			pTarget->GetCoords(&coords);

			// function to check the exact range
			auto actionIfInRange = [&](CellClass* pCell) -> bool {

				// get the cell contents
				for(ObjectClass* pContent = pCell->GetContent(); pContent; pContent = pContent->NextObject) {
					CoordStruct tmpCoords;
					pContent->GetCoords(&tmpCoords);

					// if it is near enough, do action
					if(coords.DistanceFrom(tmpCoords) <= maxDistance * 256) {
						if(action(pContent)) {
							++ret;
						} else {
							return false;
						}
					}				
				}

				return true;
			};

			if((height > 0) && ((height * (int)widthOrRange) > 0)) {
				// rectangle
				maxDistance += height;
				return forEach(cell, (int)widthOrRange, height, actionIfInRange);
			} else if(widthOrRange > 0) {
				// circle, with thick border
				return forEach(cell, widthOrRange + 1, actionIfInRange);
			}

			return ret;
		}

		template<typename T>
		class DistinctCollector {
		public:
			std::set<T, StrictWeakComparer<T> > Value;

			bool Collect(T value) {
				Value.insert(value);
				return true;
			}

			std::tr1::function<bool (T)> getCollector() {
				return [&](T obj) -> bool { return Collect(obj); };
			}

			void forEach(std::tr1::function<bool (T)> action) {
				if(action) {
					for(auto iterator = Value.begin(); iterator != Value.end(); iterator++) {
						T Obj = *iterator;
						if(!action(Obj)) {
							return;
						}
					}
				}
			}
		};
	};
};

#endif

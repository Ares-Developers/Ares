#ifndef HELPERS_ALEX_H
#define HELPERS_ALEX_H

#include <CellSpread.h>

#include <set>
#include <functional>
#include <algorithm>
#include <iterator>

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
		static std::vector<TechnoClass*> getCellSpreadItems(CoordStruct *coords, float spread, bool includeInAir=false) {
			// set of possibly affected objects. every object can be here only once.
			DistinctCollector<TechnoClass*> set;

			// the quick way. only look at stuff residing on the very cells we are affecting.
			CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
			int countCells = CellSpread::NumCells((int)(spread + 0.99));
			for(int i = 0; i < countCells; ++i) {
				CellStruct tmpCell = CellSpread::GetCell(i);
				tmpCell += cellCoords;
				CellClass *c = MapClass::Instance->GetCellAt(&tmpCell);
				for(ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
					if(TechnoClass *Techno = generic_cast<TechnoClass*>(curObj)) {
						set.insert(Techno);
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
							set.insert(Techno);
						}
					}
				}
			}

			// look closer. the final selection. put all affected items in a vector.
			std::vector<TechnoClass*> ret;
			for(auto iterator = set.begin(); iterator != set.end(); iterator++) {
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
					ret.push_back(Techno);
				}
			}

			return ret;
		}

		//! Invokes an action for every cell in a rectangle and returns the count of cells affected.
		/*!
			The area is centered on the target cell, rounding might apply. The action is never invoked
			for invalid cells, i.e. cells outside the map bounds.

			\param cell The center cell of the rectangle.
			\param width The width of the rectangle.
			\param height The height of the rectangle.
			\param action The action to invoke for each cell.

			\returns Count of cells affected.

			\author AlexB
		*/
		static int forEach(CellStruct *cell, int width, int height, const std::tr1::function<bool (CellClass*)> &action) {
			int ret = 0;

			// the coords mark the center of the area
			CellStruct Offset;
			Offset.X = (short)(cell->X - (width / 2));
			Offset.Y = (short)(cell->Y - (height / 2));

			// take a look at each cell in the rectangle
			for(short i=0; i<height; ++i) {
				for(short j=0; j<width; ++j) {
					// get the specific cell coordinates
					CellStruct Cell = {j, i};
					Cell += Offset;

					// get this cell and call the action function
					CellClass* pCell = MapClass::Instance->GetCellAt(&Cell);
					if(pCell != MapClass::InvalidCell()) {
						if(action(pCell)) {
							++ret;
						} else {
							return ret;
						}
					}
				}
			}

			return ret;
		}

		//! Invokes an action for every cell in range and returns the number of cells affected.
		/*!
			The action is never invoked for invalid cells, i.e. cells outside the map bounds.

			\param cell The center cell of the circular area.
			\param radius The range defining the area around the center cell.
			\param action The action to invoke for each cell.

			\returns Count of cells affected.

			\author AlexB
		*/
		static int forEach(CellStruct *cell, float radius, const std::tr1::function<bool (CellClass*)> &action) {
			int ret = 0;

			// check whether the cell in this square are in the circle also
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

			// get all cells in a square around the target.
			// the result is discarded, because not all cells in this square are
			// in the requested circular area, so we have to do the counting.
			int range = (int)std::floor(radius + 0.99f) * 2 + 1;
			forEach(cell, range, range, actionIfInRange);

			return ret;
		}

		//! Invokes an action for every cell in range and returns the number of cells affected.
		/*!
			The action is never invoked for invalid cells, i.e. cells outside the map bounds.

			\param cell The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each cell.

			\returns Count of cells affected. Negative values indicate input errors.

			\author AlexB
		*/
		static int forEachCellInRange(CellStruct *cell, float widthOrRange, int height, const std::tr1::function<bool (CellClass*)> &action) {
			if(height > 0) {
				if((int)widthOrRange > 0) {
					// rectangle
					return forEach(cell, (int)widthOrRange, height, action);
				}
			} else if(widthOrRange > 0) {
				// circle
				return forEach(cell, widthOrRange, action);
			}
			return -1;
		}

		//! Invokes an action for every object in range and returns the count of objects affected.
		/*!
			This method uses the exact distance between the given cell's center and each object.

			\param cell The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each cell.

			\returns Count of objects affected. Negative values indicate input errors.

			\author AlexB
		*/
		static int forEachObjectInRange(CellStruct *cell, float widthOrRange, int height, const std::tr1::function<bool (ObjectClass*)> &action) {
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

			bool bOK = false;
			if(height > 0) {
				// rectangle. adjust maximum allowed distance to include all objects:
				// any two points in a rectangle are closer to each oher than width+height.
				if((int)widthOrRange > 0) {
					maxDistance += height;
					forEach(cell, (int)widthOrRange, height, actionIfInRange);
					bOK = true;
				}
			} else if(widthOrRange > 0) {
				// enlarged circle to include more than needed. the final calculation
				// is done in the lambda.
				forEach(cell, widthOrRange + 1, actionIfInRange);
				bOK = true;
			}

			return bOK ? ret : -1;
		}

		//! Invokes an action for every cell in cell spread range and returns the count of cells affected.
		/*!
			This method uses the distance between the given cell's center and each object's cell's center.
			If a cell is close to the center, all its content objects are affected. If the cell is not close
			enough, no objects are affected.

			\param cell The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each cell.

			\returns Count of cells affected. Negative values indicate input errors.

			\author AlexB
		*/
		static int forEachCellInCellSpread(CellStruct *cell, float widthOrRange, int height, const std::tr1::function<bool (CellClass*)> &action) {
			// number of affected cells
			int ret = 0;

			// are we in rectangle-mode?
			bool isRectangle = (height > 0);

			int maxDistance = (int)std::floor(widthOrRange + 0.99f);

			// function to invoke the action for each object
			auto actionIfInRange = [&](CellClass* pCell) -> bool {
				if(!isRectangle) {
					// this distance calculation matches CellSpread < 11.
					// 11 has flaws in YR, which have not been recreated.
					CellStruct delta = pCell->MapCoords - *cell;
					int dx = std::abs(delta.X);
					int dy = std::abs(delta.Y);

					// distance is longer component plus half the shorter component
					int maxComp = std::max(dx, dy);
					int minComp = std::min(dx, dy);
					int distance = maxComp + minComp / 2;

					// continue with the next cell
					if(distance > maxDistance) {
						return true;
					}
				}

				// invoke the action
				if(action(pCell)) {
					++ret;
				} else {
					return false;
				}

				return true;
			};

			// make it a square containing the range circle
			if(!isRectangle) {
				height = maxDistance * 2 + 1;
				widthOrRange = static_cast<float>(height);
			}

			int res = forEachCellInRange(cell, widthOrRange, height, actionIfInRange);
			if(res < 0) {
				// invalid input
				ret = res;
			}

			return ret;
		}

		//! Invokes an action for every object in cell spread range and returns the count of objects affected.
		/*!
			This method uses the distance between the given cell's center and each object's cell's center.
			If a cell is close to the center, all its content objects are affected. If the cell is not close
			enough, no objects are affected.

			\param cell The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each object.

			\returns Count of objects affected. Negative values indicate input errors.

			\author AlexB
		*/
		static int forEachObjectInCellSpread(CellStruct *cell, float widthOrRange, int height, const std::tr1::function<bool (ObjectClass*)> &action) {
			int ret = 0;

			// function to invoke the action for each object
			auto actionIfInRange = [&](CellClass* pCell) -> bool {

				// invoke the action for all cell contents
				for(ObjectClass* pContent = pCell->GetContent(); pContent; pContent = pContent->NextObject) {
					if(action(pContent)) {
						++ret;
					} else {
						return false;
					}
				}

				return true;
			};

			int res = forEachCellInCellSpread(cell, widthOrRange, height, actionIfInRange);
			if(res < 0) {
				// invalid input
				ret = res;
			}

			return ret;
		}

		//! Less comparison for pointer types.
		/*!
			Dereferences the values before comparing them using std::less.

			This compares the actual objects pointed to instead of their
			arbitrary pointer values.
		*/
		template <typename T>
		struct deref_less : std::unary_function<const T, bool> {
			bool operator()(const T lhs, const T rhs) const {
				typedef std::remove_pointer<T>::type deref_type;
				return std::less<deref_type>()(*lhs, *rhs);
			}
		};

		//! Represents a set of unique items.
		/*!
			Items can be added using the insert method. Even though an item
			can be added multiple times, it is only contained once in the set.

			Use either the for_each method to call a method using each item as
			a parameter, or iterate the set through the begin and end methods.
		*/
		template<typename T>
		class DistinctCollector {
			typedef typename std::conditional<std::is_pointer<T>::value, deref_less<T>, std::less<T>>::type less_type;
			typedef std::set<T, less_type> set_type;
			set_type _set;

		public:
			bool operator() (T item) {
				insert(item);
				return true;
			}

			void insert(T value) {
				_set.insert(value);
			}

			size_t size() const {
				return _set.size();
			}

			typename set_type::const_iterator begin() const {
				return _set.begin();
			}

			typename set_type::const_iterator end() const {
				return _set.end();
			}

			int for_each(const std::tr1::function<bool (T)> &action) const {
				return std::distance(begin(), std::find_if_not(begin(), end(), action));
			}
		};
	};
};

#endif

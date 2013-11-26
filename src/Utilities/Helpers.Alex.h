#ifndef HELPERS_ALEX_H
#define HELPERS_ALEX_H

#include <CellSpread.h>
#include <Helpers/Enumerators.h>
#include <Helpers/Iterators.h>

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
			CellStruct cellCoords = MapClass::Instance->GetCellAt(*coords)->MapCoords;
			int countCells = CellSpread::NumCells((int)(spread + 0.99));
			for(int i = 0; i < countCells; ++i) {
				CellStruct tmpCell = CellSpread::GetCell(i);
				tmpCell += cellCoords;
				CellClass *c = MapClass::Instance->GetCellAt(tmpCell);
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
				CoordStruct target = Techno->GetCoords();
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

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle.
			\param height The height of the rectangle.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle,
					 false otherwise.

			\author AlexB
		*/
		template <typename T>
		static bool for_each_in_rect(const CellStruct &center, float widthOrRange, int height, const std::function<bool (T*)> &action) {
			if(height > 0) {
				int width = static_cast<int>(widthOrRange);

				if(width > 0) {
					CellRectIterator iter(center, width, height);
					iter.apply(action);
					return true;
				}
			}

			return false;
		}

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle, or the radius, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a circular area.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle
					 or circle, false otherwise.

			\author AlexB
		*/
		template <typename T>
		static bool for_each_in_rect_or_range(const CellStruct &center, float widthOrRange, int height, const std::function<bool (T*)> &action) {
			if(!for_each_in_rect(center, widthOrRange, height, action)) {
				if(height <= 0 && widthOrRange >= 0.0f) {
					CellRangeIterator iter(center, widthOrRange);
					iter.apply(action);
					return true;
				}
			}

			return false;
		}

		//! Invokes an action for every cell or every object contained on the cells.
		/*!
			action is invoked only once per cell. action can be invoked multiple times
			on other objects.

			\param center The center cell of the area.
			\param widthOrRange The width of the rectangle, or the spread, if height <= 0.
			\param height The height of the rectangle. Use 0 to create a CellSpread area.
			\param action The action to invoke for each object.

			\returns Returns true if widthOrRange and height describe a valid rectangle
					 or CellSpread range, false otherwise.

			\author AlexB
		*/
		template <typename T>
		static bool for_each_in_rect_or_spread(const CellStruct &center, float widthOrRange, int height, const std::function<bool (T*)> &action) {
			if(!for_each_in_rect(center, widthOrRange, height, action)) {
				if(height <= 0) {
					int spread = static_cast<int>(widthOrRange);

					if(spread > 0) {
						CellSpreadIterator iter(center, spread);
						iter.apply(action);
						return true;
					}
				}
			}

			return false;
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

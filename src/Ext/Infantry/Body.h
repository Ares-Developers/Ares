#ifndef Infantry_EXT_H
#define Infantry_EXT_H

#include <CCINIClass.h>
#include <InfantryClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class InfantryExt
{
public:
	typedef InfantryClass TT;

	class ExtData : public Extension<TT>
	{
	public:

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr) {
		}

		bool IsOccupant(); //!< Determines whether this InfantryClass is currently an occupant inside a BuildingClass.
	};

	static Container<InfantryExt> ExtMap;

	static eAction GetEngineerEnterEnemyBuildingAction(BuildingClass *pBld);
};

#endif

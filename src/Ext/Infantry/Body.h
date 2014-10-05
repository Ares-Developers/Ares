#ifndef INFANTRY_EXT_H
#define INFANTRY_EXT_H

#include <InfantryClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class BuildingClass;

class InfantryExt
{
public:
	using base_type = InfantryClass;

	class ExtData : public Extension<InfantryClass>
	{
	public:

		ExtData(InfantryClass* OwnerObject) : Extension<InfantryClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		bool IsOccupant(); //!< Determines whether this InfantryClass is currently an occupant inside a BuildingClass.
	};

	static Container<InfantryExt> ExtMap;

	static Action GetEngineerEnterEnemyBuildingAction(BuildingClass *pBld);
};

#endif

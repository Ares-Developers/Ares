#pragma once

#include <InfantryClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class BuildingClass;

class InfantryExt
{
public:
	using base_type = InfantryClass;

	class ExtData final : public Extension<InfantryClass>
	{
	public:

		ExtData(InfantryClass* OwnerObject) : Extension<InfantryClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		bool IsOccupant(); //!< Determines whether this InfantryClass is currently an occupant inside a BuildingClass.

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static Action GetEngineerEnterEnemyBuildingAction(BuildingClass *pBld);
};

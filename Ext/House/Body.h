#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include "../_Container.hpp"
#include <Helpers/Template.h>

#include <FactoryClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <SuperClass.h>

#include <bitset>

class HouseExt
{
	public:
	typedef HouseClass TT;

	class ExtData : public Extension<TT>
	{
		public:
			bool IonSensitive;
			bool FirewallActive;
			int FirewallRecalc;

			BuildingClass *Factory_BuildingType;
			BuildingClass *Factory_InfantryType;
			BuildingClass *Factory_VehicleType;
			BuildingClass *Factory_NavyType;
			BuildingClass *Factory_AircraftType;

			std::bitset<32> StolenTech;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			IonSensitive(0),
			FirewallActive(0),
			FirewallRecalc(0),
			Factory_BuildingType(NULL),
			Factory_InfantryType(NULL),
			Factory_VehicleType(NULL),
			Factory_NavyType(NULL),
			Factory_AircraftType(NULL),
			StolenTech(0ull)
		{
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr) {
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
		}

	};

	static Container<HouseExt> ExtMap;

	static signed int BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem);
	static signed int CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued);

	static signed int RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem);

	static signed int PrereqValidate
		(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued);

	static void Firestorm_SetState(HouseClass *pHouse, bool Active);

};

#endif

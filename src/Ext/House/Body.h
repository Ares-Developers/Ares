#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include "../_Container.hpp"
#include "../../Enum/Prerequisites.h"

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

	enum RequirementStatus {
		Forbidden = 1, // forbidden by special conditions (e.g. reqhouses) that's not likely to change in this session
		Incomplete = 2, // missing something (approp factory)
		Complete = 3, // OK
		Overridden = 4, // magic condition met, bypass prereq check
	};

	enum BuildLimitStatus {
		ReachedPermanently = -1, // remove cameo
		ReachedTemporarily = 0, // black out cameo
		NotReached = 1, // don't do anything
	};

	class ExtData : public Extension<TT>
	{
		public:
			bool IonSensitive;
			bool FirewallActive;
			int FirewallRecalc;

			int SWLastIndex;

			BuildingClass *Factory_BuildingType;
			BuildingClass *Factory_InfantryType;
			BuildingClass *Factory_VehicleType;
			BuildingClass *Factory_NavyType;
			BuildingClass *Factory_AircraftType;

			std::bitset<32> StolenTech;
			IndexBitfield<HouseClass*> RadarPersist; 

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			IonSensitive(0),
			FirewallActive(0),
			FirewallRecalc(0),
			Factory_BuildingType(nullptr),
			Factory_InfantryType(nullptr),
			Factory_VehicleType(nullptr),
			Factory_NavyType(nullptr),
			Factory_AircraftType(nullptr),
			SWLastIndex(0),
			RadarPersist(),
			StolenTech(0ull)
		{
		};

		virtual ~ExtData() {

		}

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
		}

		void SetFirestormState(bool Active);

		bool CheckBasePlanSanity();

		int GetSurvivorDivisor() const;
		InfantryTypeClass* GetCrew() const;
		InfantryTypeClass* GetEngineer() const;
		InfantryTypeClass* GetTechnician() const;
		InfantryTypeClass* GetDisguise() const;
	};

	static Container<HouseExt> ExtMap;

	static signed int BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem);
	static BuildLimitStatus CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued);

	static RequirementStatus RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesListed(const Prereqs::BTypeIter &List, TechnoTypeClass *pItem);

	static bool HasNeededFactory(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool FactoryForObjectExists(HouseClass *pHouse, TechnoTypeClass *pItem);

	static bool IsAnyFirestormActive;
	static bool UpdateAnyFirestormActive();

	static signed int PrereqValidate
		(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued);

	static SideClass* GetSide(HouseClass* pHouse);
};

#endif

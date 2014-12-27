#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include "../_Container.hpp"
#include "../../Enum/Prerequisites.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <HouseClass.h>

#include <bitset>

class FactoryClass;
class SuperClass;
class SideClass;
class SuperWeaponTypeClass;

class HouseExt
{
public:
	using base_type = HouseClass;

	enum class RequirementStatus {
		Forbidden = 1, // forbidden by special conditions (e.g. reqhouses) that's not likely to change in this session
		Incomplete = 2, // missing something (approp factory)
		Complete = 3, // OK
		Overridden = 4, // magic condition met, bypass prereq check
	};

	enum class BuildLimitStatus {
		ReachedPermanently = -1, // remove cameo
		ReachedTemporarily = 0, // black out cameo
		NotReached = 1, // don't do anything
	};

	enum class IonCannonCloakOptions {
		RandomizeCloaked = 0,
		AgnosticToCloak = 1,
		IgnoreCloaked = 2,
		RequireCloaked = 3
	};

	class ExtData : public Extension<HouseClass>
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

		ValueableVector<HouseTypeClass *> FactoryOwners_GatheredPlansOf;

		std::vector<BuildingClass*> Academies;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject),
			IonSensitive(false),
			FirewallActive(false),
			FirewallRecalc(0),
			Factory_BuildingType(nullptr),
			Factory_InfantryType(nullptr),
			Factory_VehicleType(nullptr),
			Factory_NavyType(nullptr),
			Factory_AircraftType(nullptr),
			SWLastIndex(-1),
			RadarPersist(),
			StolenTech(0ull)
		{ }

		virtual ~ExtData();

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		void SetFirestormState(bool Active);

		bool CheckBasePlanSanity();

		void UpdateTogglePower();

		int GetSurvivorDivisor() const;
		InfantryTypeClass* GetCrew() const;
		InfantryTypeClass* GetEngineer() const;
		InfantryTypeClass* GetTechnician() const;
		InfantryTypeClass* GetDisguise() const;

		void UpdateAcademy(BuildingClass* pAcademy, bool added);
		void ApplyAcademy(TechnoClass* pTechno, AbstractType considerAs) const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static Container<HouseExt> ExtMap;
	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static signed int BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem);
	static BuildLimitStatus CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued);

	static RequirementStatus RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesListed(const Prereqs::BTypeIter &List, TechnoTypeClass *pItem);

	static bool HasNeededFactory(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool FactoryForObjectExists(HouseClass *pHouse, TechnoTypeClass *pItem);

	static bool CheckFactoryOwners(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool CheckFactoryOwner(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool CheckForbiddenFactoryOwner(HouseClass *pHouse, TechnoTypeClass *pItem);

	static bool IsAnyFirestormActive;
	static bool UpdateAnyFirestormActive();

	static signed int PrereqValidate
		(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued);

	static SideClass* GetSide(HouseClass* pHouse);

	static HouseClass* GetHouseKind(OwnerHouseKind kind, bool allowRandom,
		HouseClass* pDefault, HouseClass* pInvoker = nullptr,
		HouseClass* pKiller = nullptr, HouseClass* pVictim = nullptr);

	static TechnoClass* PickIonCannonTarget(HouseClass* pOwner, HouseClass* pVictim,
		SuperWeaponTypeClass* pSWType = nullptr,
		IonCannonCloakOptions cloak = IonCannonCloakOptions::RandomizeCloaked);
};

#endif

#pragma once

#include "../_Container.hpp"
#include "../../Enum/Prerequisites.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Iterator.h"
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

	enum class FactoryState {
		NoFactory = 0, // there is no factory building for this
		Unpowered = 1, // there is a factory building, but it is offline
		Available = 2 // at least one factory building is as online as required
	};

	class ExtData final : public Extension<HouseClass>
	{
	public:
		// data read from the INI (type-like)
		Nullable<bool> Degrades;

		// data for the house instance
		bool IonSensitive;
		bool FirewallActive;

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

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		void SetFirestormState(bool active);

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

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override {
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			return abs != AbstractType::Building;
		}
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static int CountOwnedNowTotal(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static signed int BuildLimitRemaining(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static BuildLimitStatus CheckBuildLimit(HouseClass const* pHouse, TechnoTypeClass const* pItem, bool includeQueued);

	static RequirementStatus RequirementsMet(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static bool PrerequisitesMet(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static bool PrerequisitesListed(const Prereqs::BTypeIter &List, TechnoTypeClass const* pItem);

	static FactoryState HasFactory(
		HouseClass const* pHouse, TechnoTypeClass const* pItem,
		bool requirePower);

	static bool CheckFactoryOwners(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static bool CheckFactoryOwner(HouseClass const* pHouse, TechnoTypeClass const* pItem);
	static bool CheckForbiddenFactoryOwner(HouseClass const* pHouse, TechnoTypeClass const* pItem);

	static bool IsAnyFirestormActive;
	static bool UpdateAnyFirestormActive(bool lastChange);

	static signed int PrereqValidate(
		HouseClass const* pHouse, TechnoTypeClass const* pItem,
		bool buildLimitOnly, bool includeQueued);

	static bool IsDisabledFromShell(
		HouseClass const* pHouse, BuildingTypeClass const* pItem);

	static size_t FindOwnedIndex(
		HouseClass const* pHouse, int idxParentCountry,
		Iterator<TechnoTypeClass const*> items, size_t start = 0);

	static size_t FindBuildableIndex(
		HouseClass const* pHouse, int idxParentCountry,
		Iterator<TechnoTypeClass const*> items, size_t start = 0);

	template <typename T>
	static T* FindOwned(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindOwnedIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	template <typename T>
	static T* FindBuildable(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindBuildableIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	static SideClass* GetSide(HouseClass* pHouse);

	static HouseClass* GetHouseKind(OwnerHouseKind kind, bool allowRandom,
		HouseClass* pDefault, HouseClass* pInvoker = nullptr,
		HouseClass* pKiller = nullptr, HouseClass* pVictim = nullptr);

	// temporary storage for the 100-unit bug fix
	static std::vector<int> AIProduction_CreationFrames;
	static std::vector<int> AIProduction_Values;
	static std::vector<int> AIProduction_BestChoices;
};

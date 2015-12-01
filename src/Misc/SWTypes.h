#pragma once

#include "../Ext/SWType/Body.h"
#include "../Utilities/Enums.h"

#include <SuperClass.h>

#include <vector>

enum class SWStateMachineIdentifier : unsigned int {
	Invalid = 0xFFFFFFFFu,
	UnitDelivery = 0,
	ChronoWarp = 1,
	PsychicDominator = 2
};

class SWTypeExt;

struct TargetingData {
	TargetingData(SWTypeExt::ExtData* pTypeExt, HouseClass* pOwner) noexcept;
	~TargetingData() noexcept;

	struct LaunchSite
	{
		BuildingClass* Building;
		CellStruct Center;
		double MinRange;
		double MaxRange;
	};

	struct RangedItem
	{
		int RangeSqr;
		CellStruct Center;
	};

	SWTypeExt::ExtData* TypeExt;
	HouseClass* Owner;
	bool NeedsLaunchSite;
	bool NeedsDesignator;
	std::vector<LaunchSite> LaunchSites;
	std::vector<RangedItem> Designators;
	std::vector<RangedItem> Inhibitors;
};

// New SW Type framework. See SWTypes/*.h for examples of implemented ones. Don't touch yet, still WIP.
class NewSWType
{
	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType) {
		pType->SetTypeIndex(static_cast<int>(Array.size()));
		Array.emplace_back(std::move(pType));
	}

	int TypeIndex{ -1 };

public:
	virtual ~NewSWType() = default;

	std::unique_ptr<const TargetingData> GetTargetingData(SWTypeExt::ExtData* pSWType, HouseClass* pOwner) const;

	bool CanFireAt(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& cell, bool manual) const;

	virtual bool CanFireAt(TargetingData const& data, CellStruct const& cell, bool manual) const;

	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) {
		return false;
	}

	virtual bool Activate(SuperClass* pSW, const CellStruct &Coords, bool IsPlayer) = 0;

	virtual void Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer) {
	}

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) {
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) {
	}

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Warhead;
	}

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Anim;
	}

	virtual int GetSound(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Sound;
	}

	virtual int GetDamage(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Damage;
	}

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const {
		return pData->SW_Range;
	}

	virtual const char* GetTypeString() const {
		return nullptr;
	}

	int GetTypeIndex() const {
		return TypeIndex;
	}

	virtual bool HandlesType(SuperWeaponType type) const {
		return false;
	}

	virtual SuperWeaponFlags Flags() const {
		return SuperWeaponFlags::None;
	}

protected:
	virtual void SetTypeIndex(int const index) {
		this->TypeIndex = index;
	}

	virtual bool IsLaunchSite(SWTypeExt::ExtData *pSWType, BuildingClass* pBuilding) const;

	virtual std::pair<double, double> GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding = nullptr) const;

	bool HasLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const;

	bool IsLaunchSiteEligible(SWTypeExt::ExtData* pSWType, const CellStruct &Coords, BuildingClass* pBuilding, bool ignoreRange) const;

	virtual bool IsDesignator(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno) const;

	bool HasDesignator(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const;

	bool IsDesignatorEligible(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords, TechnoClass* pTechno) const;

	virtual bool IsInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno) const;

	bool HasInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const;

	bool IsInhibitorEligible(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords, TechnoClass* pTechno) const;

public:
	// static methods
	static void Init();

	static NewSWType* GetNthItem(SuperWeaponType i) {
		return Array[static_cast<size_t>(i) - SWTypeExt::FirstCustomType].get();
	}

	static SuperWeaponType FindIndex(const char* pType);

	static SuperWeaponType FindHandler(SuperWeaponType Type);

	static bool LoadGlobals(AresStreamReader& Stm);

	static bool SaveGlobals(AresStreamWriter& Stm);
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine {
	static std::vector<std::unique_ptr<SWStateMachine>> Array;

public:
	SWStateMachine()
		: Type(nullptr), Super(nullptr), Coords(), Clock()
	{ }

	SWStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: Type(pSWType), Super(pSuper), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() {
	}

	virtual bool Finished() {
		return Clock.Completed();
	}

	virtual void Update() {
	};

	virtual void InvalidatePointer(void *ptr, bool remove) {
	};

	int TimePassed() {
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	SWTypeExt::ExtData * FindExtData() const {
		return SWTypeExt::ExtMap.Find(this->Super->Type);
	}

	virtual SWStateMachineIdentifier GetIdentifier() const = 0;

	virtual bool Load(AresStreamReader &Stm, bool RegisterForChange);

	virtual bool Save(AresStreamWriter &Stm) const;

	// static methods
	static void Register(std::unique_ptr<SWStateMachine> Machine) {
		if(Machine) {
			Array.push_back(std::move(Machine));
		}
	}

	static void UpdateAll();

	static void PointerGotInvalid(void *ptr, bool remove);

	static void Clear();

	static bool LoadGlobals(AresStreamReader& Stm);

	static bool SaveGlobals(AresStreamWriter& Stm);

protected:
	TimerStruct Clock;
	SuperClass* Super;
	NewSWType* Type;
	CellStruct Coords;
};

class UnitDeliveryStateMachine : public SWStateMachine {
public:
	UnitDeliveryStateMachine()
		: SWStateMachine()
	{ }

	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{ }

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::UnitDelivery;
	}

	void PlaceUnits();
};

class ChronoWarpStateMachine : public SWStateMachine {
public:
	struct ChronoWarpContainer {
	public:
		BuildingClass* building;
		CellStruct target;
		CoordStruct origin;
		bool isVehicle;

		ChronoWarpContainer(BuildingClass* pBld, const CellStruct& target, const CoordStruct& origin, bool isVehicle) :
			building(pBld),
			target(target),
			origin(origin),
			isVehicle(isVehicle)
		{
		}

		ChronoWarpContainer() = default;

		bool operator == (const ChronoWarpContainer &other) const {
			return this->building == other.building;
		}
	};

	ChronoWarpStateMachine()
		: SWStateMachine(), Buildings(), Duration(0)
	{ }

	ChronoWarpStateMachine(int Duration, const CellStruct& XY, SuperClass* pSuper, NewSWType* pSWType, DynamicVectorClass<ChronoWarpContainer> Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType), Buildings(std::move(Buildings)), Duration(Duration)
	{ }

	virtual void Update();

	virtual void InvalidatePointer(void *ptr, bool remove);

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::ChronoWarp;
	}

	virtual bool Load(AresStreamReader &Stm, bool RegisterForChange) override;

	virtual bool Save(AresStreamWriter &Stm) const override;

protected:
	DynamicVectorClass<ChronoWarpContainer> Buildings;
	int Duration;
};

class PsychicDominatorStateMachine : public SWStateMachine {
public:
	PsychicDominatorStateMachine()
		: SWStateMachine(), Deferment(0)
	{}

	PsychicDominatorStateMachine(CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType), Deferment(0)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		// the initial deferment
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment.Get(0);

		// make the game happy
		PsyDom::Owner = pSuper->Owner;
		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	};

	virtual void Update();

	virtual SWStateMachineIdentifier GetIdentifier() const override {
		return SWStateMachineIdentifier::PsychicDominator;
	}

	virtual bool Load(AresStreamReader &Stm, bool RegisterForChange) override;

	virtual bool Save(AresStreamWriter &Stm) const override;

protected:
	int Deferment;
};

template <>
struct Savegame::ObjectFactory<SWStateMachine> {
	std::unique_ptr<SWStateMachine> operator() (AresStreamReader &Stm) const {
		SWStateMachineIdentifier type = SWStateMachineIdentifier::Invalid;
		if(Stm.Load(type)) {
			switch(type) {
			case SWStateMachineIdentifier::UnitDelivery:
				return std::make_unique<UnitDeliveryStateMachine>();
			case SWStateMachineIdentifier::ChronoWarp:
				return std::make_unique<ChronoWarpStateMachine>();
			case SWStateMachineIdentifier::PsychicDominator:
				return std::make_unique<PsychicDominatorStateMachine>();
			case SWStateMachineIdentifier::Invalid:
			default:
				Debug::FatalErrorAndExit("SWStateMachineType %d not recognized.",
					static_cast<unsigned int>(type));
			}
		}

		return nullptr;
	}
};

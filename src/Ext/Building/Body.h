#pragma once

#include <BuildingClass.h>

#include "../BuildingType/Body.h"
#include "../_Container.hpp"
#include "../../Ares.h"
#include "../../Utilities/Constructs.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

class SuperClass;

class BuildingExt
{
public:
	using base_type = BuildingClass;

	class ExtData;

	class cPrismForwarding {
	public:
		BuildingExt::ExtData* Owner;
		DynamicVectorClass<cPrismForwarding*> Senders;		//the prism towers that are forwarding to this one
		cPrismForwarding* SupportTarget;			//what tower am I sending to?
		int PrismChargeDelay;					//current delay charge
		double ModifierReserve;					//current modifier reservoir
		int DamageReserve;					//current flat reservoir

		// constructor
		cPrismForwarding(BuildingExt::ExtData* pOwner) : Owner(pOwner),
			Senders(),
			SupportTarget(nullptr),
			PrismChargeDelay(0),
			ModifierReserve(0.0),
			DamageReserve(0)
		{ }

		~cPrismForwarding() {
			this->RemoveFromNetwork(true);
		}

		BuildingClass* GetOwner() const {
			return this->Owner->OwnerObject();
		}

		int AcquireSlaves_MultiStage(cPrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
		int AcquireSlaves_SingleStage(cPrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
		bool ValidateSupportTower(cPrismForwarding* TargetTower, cPrismForwarding* SlaveTower);
		void SetChargeDelay(int LongestChain);
		void SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD* LongestCDelay, DWORD* LongestFDelay);
		void SetChargeDelay_Set(int chain, DWORD const* LongestCDelay, DWORD const* LongestFDelay, int LongestChain);
		void RemoveFromNetwork(bool bCease);
		void SetSupportTarget(cPrismForwarding* pTargetTower);
		void RemoveAllSenders();

		bool Load(AresStreamReader &Stm, bool RegisterForChange);

		bool Save(AresStreamWriter &Stm) const;
	};


	class ExtData final : public Extension<BuildingClass>
	{
	private:

	public:
		HouseClass* OwnerBeforeRaid; //!< Contains the house which owned this building prior to it being raided and turned over to the raiding party.
		bool isCurrentlyRaided; //!< Whether this building is currently occupied by someone not the actual owner of the structure.
		bool ignoreNextEVA; //!< This is used when returning raided buildings, to decide whether to play EVA announcements about building capture.

		bool FreeUnits_Done; //!< Prevent free units and aircraft to be created multiple times. Set when the free units have been granted.
		bool AboutToChronoshift; //!< This building is going to be shifted. It should not be attacked with temporal weapons now. Otherwise it would disappear.

		bool InfiltratedBy(HouseClass *Enterer);
		cPrismForwarding PrismForwarding;

		AresMap<TechnoClass*, bool> RegisteredJammers; //!< Set of Radar Jammers which have registered themselves to be in range of this building. (Related to issue #305)

		int SensorArrayActiveCounter;

		bool SecretLab_Placed;

		bool TogglePower_HasPower;

		TimerStruct CashUpgradeTimers[3];

		VectorClass<int> DockReloadTimers;

	public:
		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject),
			OwnerBeforeRaid(nullptr),
			isCurrentlyRaided(false),
			ignoreNextEVA(false),
			PrismForwarding(this),
			FreeUnits_Done(false),
			AboutToChronoshift(false),
			SecretLab_Placed(false),
			TogglePower_HasPower(true),
			SensorArrayActiveCounter(0)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		// related to Advanced Rubble
		bool RubbleYell(bool beingRepaired = false); // This function triggers back and forth between rubble states.
		void KickOutOfRubble();

		// related to trench traversal
		bool canTraverseTo(BuildingClass* targetBuilding); // Returns true if people can move from the current building to the target building, otherwise false.
		void doTraverseTo(BuildingClass* targetBuilding); // This function moves as many occupants as possible from the current to the target building.
		bool sameTrench(BuildingClass* targetBuilding); // Checks if both buildings are of the same trench kind.

		// related to linkable buildings
		bool isLinkable(); //!< Returns true if this is a structure that can be linked to other structures, like a wall, fence, or trench. This is used to quick-check if the game has to look for linkable buildings in the first place. \sa canLinkTo()
		bool canLinkTo(BuildingClass* targetBuilding); //!< Checks if the current building can link to the given target building. \param targetBuilding the building to check for compatibility. \return true if the two buildings can be linked. \sa isLinkable()

		// related to raidable buildings
		void evalRaidStatus(); //!< Checks if the building is empty but still marked as raided, and returns the building to its previous owner, if so.

		void UpdateFirewall(bool changedState = true);
		void UpdateFirewallLinks();
		void ImmolateVictims();
		bool ImmolateVictim(ObjectClass* Victim, bool destroy = true);

		bool ReverseEngineer(TechnoClass * Victim); //!< Returns true if Victim wasn't buildable and now should be

		void KickOutClones(TechnoClass * Production);

		void UpdateSensorArray();
		void UpdateSecretLab();

		size_t GetSuperWeaponCount() const;
		bool HasSuperWeapon() const;
		bool HasSuperWeapon(int index, bool withUpgrades) const;
		int GetSuperWeaponIndex(size_t index) const;
		SuperClass* GetSuperWeapon(size_t index) const;
		int GetFirstSuperWeaponIndex() const;
		SuperClass* GetFirstSuperWeapon() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static void Clear();

	static DWORD GetFirewallFlags(BuildingClass *pThis);

	//static void ExtendFirewall(BuildingClass *pThis, CellStruct Center, HouseClass *Owner); // replaced by generic buildLines
	static void buildLines(BuildingClass*, CellStruct, HouseClass*); // Does the actual line-building, using isLinkable() and canLinkTo().

	static void UpdateDisplayTo(BuildingClass *pThis);

	static signed int GetImageFrameIndex(BuildingClass *pThis);

	static void KickOutHospitalArmory(BuildingClass *pThis);

	static CoordStruct GetCenterCoords(BuildingClass* pThis, bool includeBib = false);

	static std::vector<CellStruct> TempFoundationData1;
	static std::vector<CellStruct> TempFoundationData2;

	static std::vector<CellStruct> TempCoveredCellsData;

	static DWORD FoundationLength(CellStruct const* pFoundation);

	static const std::vector<CellStruct>& GetCoveredCells(
		BuildingClass* pThis, CellStruct mainCoords, int shadowHeight);

	static bool IsActiveFirestormWall(BuildingClass* pBuilding, HouseClass const* pIgnore = nullptr);

	// if it is a factory, update the queues for the owner
	static void UpdateFactoryQueues(BuildingClass const* pBuilding);
};

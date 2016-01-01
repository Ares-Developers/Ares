#pragma once

#include <algorithm>

#include "../../Misc/AttachEffect.h"
#include "../../Misc/JammerClass.h"
#include "../../Misc/PoweredUnitClass.h"

#include "../../Utilities/Constructs.h"
#include "../../Utilities/Enums.h"

#include "../_Container.hpp"

class AircraftClass;
class AlphaShapeClass;
class BuildingLightClass;
class EBolt;
class HouseClass;
class HouseTypeClass;
class InfantryClass;
struct SHPStruct;
class TemporalClass;
class WarheadTypeClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		// weapon slots fsblargh
		BYTE idxSlot_Wave;
		BYTE idxSlot_Beam;
		BYTE idxSlot_Warp;
		BYTE idxSlot_Parasite;

		bool Survivors_Done;

		TimerStruct CloakSkipTimer;
		SHPStruct * Insignia_Image;

		BuildingClass *GarrisonedIn; // when infantry garrisons a building, we need a fast way to find said building when damage forwarding kills it

		AnimClass *EMPSparkleAnim;
		Mission EMPLastMission;

		bool ShadowDrawnManually;

		bool DriverKilled;

		int HijackerHealth;
		HouseClass* HijackerHouse;
		float HijackerVeterancy;

		// 305 Radar Jammers
		std::unique_ptr<JammerClass> RadarJam;

		// issue #617 powered units
		std::unique_ptr<PoweredUnitClass> PoweredUnit;

		//#1573, #1623, #255 Stat-modifiers/ongoing animations
		std::vector<AttachEffectClass> AttachedEffects;
		bool AttachEffects_RecreateAnims;

		//stuff for #1623
		bool AttachedTechnoEffect_isset;
		int AttachedTechnoEffect_Delay;

		//crate fields
		double Crate_FirepowerMultiplier;
		double Crate_ArmorMultiplier;
		double Crate_SpeedMultiplier;
		bool Crate_Cloakable;

		TemporalClass * MyOriginalTemporal;

		EBolt * MyBolt;

		HouseTypeClass* OriginalHouseType;

		BuildingLightClass* Spotlight;

		OptionalStruct<bool, true> AltOccupation; // if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members

		bool PayloadCreated;

		SuperClass* SuperWeapon; // the super weapon somehow attached to this (not provided by this)
		AbstractClass* SuperTarget; // the attached super weapon's target (if any)

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject),
			idxSlot_Wave(0),
			idxSlot_Beam(0),
			idxSlot_Warp(0),
			idxSlot_Parasite(0),
			Survivors_Done(0),
			Insignia_Image(nullptr),
			GarrisonedIn(nullptr),
			HijackerHealth(-1),
			HijackerHouse(nullptr),
			HijackerVeterancy(0.0f),
			DriverKilled(false),
			EMPSparkleAnim(nullptr),
			EMPLastMission(Mission::None),
			ShadowDrawnManually(false),
			RadarJam(nullptr),
			PoweredUnit(nullptr),
			MyOriginalTemporal(nullptr),
			MyBolt(nullptr),
			Spotlight(nullptr),
			AltOccupation(),
			PayloadCreated(false),
			SuperWeapon(nullptr),
			SuperTarget(nullptr),
			OriginalHouseType(nullptr),
			AttachEffects_RecreateAnims(false),
			AttachedTechnoEffect_isset(false),
			AttachedTechnoEffect_Delay(0),
			Crate_FirepowerMultiplier(1.0),
			Crate_ArmorMultiplier(1.0),
			Crate_SpeedMultiplier(1.0),
			Crate_Cloakable(false)
		{ }

		virtual ~ExtData() {
			this->SetSpotlight(nullptr);
		}

		// when any pointer in the game expires, this is called - be sure to tell everyone we own to invalidate it
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
			AnnounceInvalidPointer(this->GarrisonedIn, ptr);
			this->InvalidateAttachEffectPointer(ptr);
			AnnounceInvalidPointer(this->MyOriginalTemporal, ptr);
			AnnounceInvalidPointer(this->Spotlight, ptr);
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		bool IsOperated() const;
		bool IsPowered() const;

		AresAction GetActionHijack(TechnoClass* pTarget) const;
		bool PerformActionHijack(TechnoClass* pTarget) const;

		unsigned int AlphaFrame(const SHPStruct* Image) const;

		bool DrawVisualFX() const;

		UnitTypeClass* GetUnitType() const;

		bool IsDeactivated() const;

		Action GetDeactivatedAction(ObjectClass* pHovered = nullptr) const;

		void InvalidateAttachEffectPointer(void *ptr);

		void RefineTiberium(float amount, int idxType);
		void DepositTiberium(float amount, float bonus, int idxType);

		bool IsCloakable(bool allowPassive) const;
		bool CloakAllowed() const;
		bool CloakDisallowed(bool allowPassive) const;
		bool CanSelfCloakNow() const;

		void SetSpotlight(BuildingLightClass* pSpotlight);

		bool AcquireHunterSeekerTarget() const;

		void RecalculateStats();

		int GetSelfHealAmount() const;

		void CreateInitialPayload();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override {
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch(abs) {
			case AbstractType::Building:
			case AbstractType::BuildingLight:
			case AbstractType::Temporal:
			case AbstractType::Anim:
			case AbstractType::Aircraft:
			case AbstractType::Infantry:
			case AbstractType::Unit:
				return false;
			default:
				return true;
			}
		}

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static AresMap<ObjectClass*, AlphaShapeClass*> AlphaExt;

	static BuildingLightClass * ActiveBuildingLight;

	static bool NeedsRegap;

	static void SpawnSurvivors(FootClass *pThis, TechnoClass *pKiller, bool Select, bool IgnoreDefenses);
	static bool EjectSurvivor(FootClass *Survivor, CoordStruct loc, bool Select);
	static void EjectPassengers(FootClass *, int);
	static CoordStruct GetPutLocation(CoordStruct, int);
	static bool EjectRandomly(FootClass*, CoordStruct const &, int, bool);
	// If available, removes the hijacker from its victim and creates an InfantryClass instance.
	static InfantryClass* RecoverHijacker(FootClass *pThis);

	static void StopDraining(TechnoClass *Drainer, TechnoClass *Drainee);

	static bool CreateWithDroppod(FootClass *Object, const CoordStruct& XYZ);

	static void TransferIvanBomb(TechnoClass *From, TechnoClass *To);
	static void TransferAttachedEffects(TechnoClass *From, TechnoClass *To);
	static void TransferOriginalOwner(TechnoClass* pFrom, TechnoClass* pTo);

	static void FreeSpecificSlave(TechnoClass *Slave, HouseClass *Affector);
	static void DetachSpecificSpawnee(TechnoClass *Spawnee, HouseClass *NewSpawneeOwner);
	static bool CanICloakByDefault(TechnoClass *pTechno);

	static bool IsCloaked(TechnoClass* pTechno);

	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller = nullptr, HouseClass* pKillerHouse = nullptr, WarheadTypeClass* pWarhead = nullptr);

	static bool SpawnVisceroid(CoordStruct &crd, ObjectTypeClass* pType, int chance, bool ignoreTibDeathToVisc);

	static void DecreaseAmmo(
		TechnoClass* pThis, WeaponTypeClass const* pWeapon = nullptr);
/*
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
*/
};

#pragma once

#include <TechnoTypeClass.h>
#include <ParticleSystemTypeClass.h>

#include "../../Ares.h"
#include "../_Container.hpp"
#include "../../Utilities/Template.h"
#include "../../Utilities/Constructs.h"
#include "../../Misc/AttachEffect.h"

#include <bitset>

class BuildingTypeClass;
class HouseTypeClass;
class VocClass;
class VoxClass;
class WarheadTypeClass;

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	enum class SpotlightAttachment {
		Body, Turret, Barrel
	};

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		DynamicVectorClass<InfantryTypeClass *> Survivors_Pilots;
		Promotable<int> Survivors_PilotChance;
		Promotable<int> Survivors_PassengerChance;
		// new on 28.09.09 for #631
		int Survivors_PilotCount; //!< Defines the number of pilots inside this vehicle if Crewed=yes; maximum number of pilots who can survive. Defaults to 0 if Crewed=no; defaults to 1 if Crewed=yes. // NOTE: Flag in INI is called Survivor.Pilots
		Nullable<int> Crew_TechnicianChance;
		Nullable<int> Crew_EngineerChance;

		// animated cameos
		//int Cameo_Interval;
		//int Cameo_CurrentFrame;
		//TimerStruct Cameo_Timer;

		std::vector<DynamicVectorClass<int>> PrerequisiteLists;
		DynamicVectorClass<int> PrerequisiteNegatives;
		DWORD PrerequisiteTheaters;

		mutable OptionalStruct<bool, true> GenericPrerequisite;

		// new secret lab
		DWORD Secret_RequiredHouses;
		DWORD Secret_ForbiddenHouses;

		bool Is_Deso;
		bool Is_Deso_Radiation;
		bool Is_Cow;
		bool Is_Spotlighted;

		// spotlights
		int Spot_Height;
		int Spot_Distance;
		SpotlightAttachment Spot_AttachedTo;
		bool Spot_DisableR;
		bool Spot_DisableG;
		bool Spot_DisableB;
		bool Spot_Reverse;

		bool Is_Bomb;

		// these are not implemented at all yet
		//DynamicVectorClass<WeaponStruct> Weapons;
		//DynamicVectorClass<WeaponStruct> EliteWeapons;

		Promotable<SHPStruct *> Insignia;
		Nullable<bool> Insignia_ShowEnemy;

		Valueable<AnimTypeClass*> Parachute_Anim;

		// new on 08.11.09 for #342 (Operator=)
		InfantryTypeClass * Operator; //!< Saves a pointer to an InfantryType required to be a passenger of this unit in order for it to work. Defaults to NULL. \sa TechnoClass_Update_CheckOperators, bool IsAPromiscuousWhoreAndLetsAnyoneRideIt
		bool IsAPromiscuousWhoreAndLetsAnyoneRideIt; //!< If this is true, Operator= is not checked, and the object will work with any passenger, provided there is one. \sa InfantryTypeClass * Operator

		ValueableVector<TechnoTypeClass*> InitialPayload_Types;
		ValueableVector<int> InitialPayload_Nums;

		CustomPalette CameoPal;

		std::bitset<32> RequiredStolenTech;

		Nullable<bool> ImmuneToEMP;
		bool VeteranAbilityEMPIMMUNE;
		bool EliteAbilityEMPIMMUNE;
		int EMP_Threshold;
		Valueable<double> EMP_Modifier;
		Nullable<AnimTypeClass*> EMP_Sparkles;

		Valueable<double> IronCurtain_Modifier;
		Valueable<double> ForceShield_Modifier;

		Valueable<bool> Chronoshift_Allow;
		Valueable<bool> Chronoshift_IsVehicle;

		// new on 05.04.10 for #733 (KillDriver/"Jarmen Kell")
		Valueable<bool> ProtectedDriver; //!< Whether the driver of this vehicle cannot be killed, i.e. whether this vehicle is immune to KillDriver. Request #733.
		Nullable<double> ProtectedDriver_MinHealth; //!< The health level the unit has to be below so the driver can be killed
		Valueable<bool> CanDrive; //!< Whether this TechnoType can act as the driver of vehicles whose driver has been killed. Request #733.

		Valueable<bool> AlternateTheaterArt;

		Valueable<bool> PassengersGainExperience;
		Valueable<bool> ExperienceFromPassengers;
		Valueable<double> PassengerExperienceModifier;
		Valueable<double> MindControlExperienceSelfModifier;
		Valueable<double> MindControlExperienceVictimModifier;
		Valueable<double> SpawnExperienceOwnerModifier;
		Valueable<double> SpawnExperienceSpawnModifier;
		Valueable<bool> ExperienceFromAirstrike;
		Valueable<double> AirstrikeExperienceModifier;

		ValueableIdx<VocClass> VoiceRepair;
		NullableIdx<VocClass> VoiceAirstrikeAttack;
		NullableIdx<VocClass> VoiceAirstrikeAbort;

		ValueableIdx<VocClass> HijackerEnterSound;
		ValueableIdx<VocClass> HijackerLeaveSound;
		Valueable<int> HijackerKillPilots;
		Valueable<bool> HijackerBreakMindControl;
		Valueable<bool> HijackerAllowed;
		Valueable<bool> HijackerOneTime;

		Valueable<UnitTypeClass *> WaterImage;

		NullableIdx<VocClass> CloakSound;
		NullableIdx<VocClass> DecloakSound;
		Valueable<bool> CloakPowered;
		Valueable<bool> CloakDeployed;
		Valueable<bool> CloakAllowed;
		Nullable<int> CloakStages;

		Valueable<bool> SensorArray_Warn;

		AresPCXFile CameoPCX;
		AresPCXFile AltCameoPCX;

		AresFixedString<0x20> GroupAs;

		AresMap<HouseClass const*, bool> ReversedByHouses;
		Valueable<bool> CanBeReversed;

		// issue #305
		Valueable<int> RadarJamRadius; //!< Distance in cells to scan for & jam radars

		// issue #1208
		Valueable<bool> PassengerTurret; //!< Whether this unit's turret changes based on the number of people in its passenger hold.

		// issue #617
		ValueableVector<BuildingTypeClass*> PoweredBy;  //!< The buildingtype this unit is powered by or NULL.

		//issue #1623
		AttachEffectTypeClass AttachedTechnoEffect; //The AttachedEffect which should been on the Techno from the start.

		ValueableVector<BuildingTypeClass const*> BuiltAt;
		Valueable<bool> Cloneable;
		ValueableVector<BuildingTypeClass *> ClonedAt;

		Nullable<bool> CarryallAllowed;
		Nullable<int> CarryallSizeLimit;

		Valueable<bool> ImmuneToAbduction; //680, 1362

		ValueableVector<HouseTypeClass *> FactoryOwners;
		ValueableVector<HouseTypeClass *> ForbiddenFactoryOwners;
		Valueable<bool> FactoryOwners_HaveAllPlans;

		Valueable<bool> GattlingCyclic;

		Nullable<bool> Crashable;
		Valueable<bool> CrashSpin;
		Valueable<int> AirRate;

		Valueable<bool> CivilianEnemy;

		// custom missiles
		Valueable<bool> IsCustomMissile;
		Valueable<RocketStruct> CustomMissileData;
		Valueable<WarheadTypeClass*> CustomMissileWarhead;
		Valueable<WarheadTypeClass*> CustomMissileEliteWarhead;
		Valueable<AnimTypeClass*> CustomMissileTakeoffAnim;
		Valueable<AnimTypeClass*> CustomMissileTrailerAnim;
		Valueable<int> CustomMissileTrailerSeparation;
		Valueable<WeaponTypeClass*> CustomMissileWeapon;
		Valueable<WeaponTypeClass*> CustomMissileEliteWeapon;

		// tiberium related
		Nullable<bool> TiberiumProof;
		Nullable<bool> TiberiumRemains;
		Valueable<bool> TiberiumSpill;
		Nullable<int> TiberiumTransmogrify;

		// refinery and storage related
		Valueable<bool> Refinery_UseStorage;

		ValueableIdx<VoxClass> EVA_UnitLost;

		Valueable<bool> Drain_Local;
		Valueable<int> Drain_Amount;

		// smoke when damaged
		Nullable<int> SmokeChanceRed;
		Nullable<int> SmokeChanceDead;
		Valueable<AnimTypeClass*> SmokeAnim;

		// hunter seeker
		Nullable<int> HunterSeekerDetonateProximity;
		Nullable<int> HunterSeekerDescendProximity;
		Nullable<int> HunterSeekerAscentSpeed;
		Nullable<int> HunterSeekerDescentSpeed;
		Nullable<int> HunterSeekerEmergeSpeed;
		Valueable<bool> HunterSeekerIgnore;

		// super weapon
		Nullable<int> DesignatorRange;
		Nullable<int> InhibitorRange;

		// particles
		Nullable<bool> DamageSparks;

		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSmoke;
		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSparks;

		// berserk
		Nullable<double> BerserkROFMultiplier;

		// assault options
		Valueable<int> AssaulterLevel;

		// crushing
		Valueable<bool> OmniCrusher_Aggressive;
		Promotable<int> CrushDamage;
		Nullable<WarheadTypeClass*> CrushDamageWarhead;

		Nullable<double> ReloadRate;

		Valueable<int> ReloadAmount;
		Nullable<int> EmptyReloadAmount;

		Valueable<bool> Saboteur;

		Valueable<bool> CanPassiveAcquire_Guard;
		Valueable<bool> CanPassiveAcquire_Cloak;

		Nullable<double> SelfHealing_Rate;
		Promotable<int> SelfHealing_Amount;
		Promotable<double> SelfHealing_Max;

		ValueableVector<TechnoTypeClass*> PassengersWhitelist;
		ValueableVector<TechnoTypeClass*> PassengersBlacklist;

		Valueable<bool> NoManualUnload;
		Valueable<bool> NoManualFire;
		Valueable<bool> NoManualEnter;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			Survivors_PilotChance(-1),
			Survivors_PassengerChance(-1),
			Survivors_PilotCount(-1),
			Crew_TechnicianChance(),
			Crew_EngineerChance(),
			PrerequisiteTheaters(0xFFFFFFFF),
			Secret_RequiredHouses(0xFFFFFFFF),
			Secret_ForbiddenHouses(0),
			Is_Deso(false),
			Is_Deso_Radiation(false),
			Is_Cow(false),
			Is_Spotlighted(false),
			Spot_Height(200),
			Spot_Distance(1024),
			Spot_AttachedTo(SpotlightAttachment::Body),
			Spot_DisableR(false),
			Spot_DisableG(false),
			Spot_DisableB(false),
			Spot_Reverse(false),
			Drain_Local(false),
			CanPassiveAcquire_Guard(true),
			CanPassiveAcquire_Cloak(true),
			SelfHealing_Amount(1),
			SelfHealing_Max(1.0),
			Drain_Amount(0),
			SmokeChanceRed(),
			SmokeChanceDead(),
			SmokeAnim(nullptr),
			HunterSeekerDetonateProximity(),
			HunterSeekerDescendProximity(),
			HunterSeekerAscentSpeed(),
			HunterSeekerDescentSpeed(),
			HunterSeekerEmergeSpeed(),
			HunterSeekerIgnore(false),
			DesignatorRange(),
			InhibitorRange(),
			Is_Bomb(false),
			Insignia(nullptr),
			Parachute_Anim(nullptr),
			Operator(nullptr),
			IsAPromiscuousWhoreAndLetsAnyoneRideIt(false),
			CameoPal(),
			RequiredStolenTech(0ull),
			Chronoshift_Allow(true),
			Chronoshift_IsVehicle(false),
			IronCurtain_Modifier(1.0),
			ForceShield_Modifier(1.0),
			EMP_Threshold(-1),
			EMP_Modifier(1.0),
			VeteranAbilityEMPIMMUNE(false),
			EliteAbilityEMPIMMUNE(false),
			ProtectedDriver(false),
			CanDrive(false),
			AlternateTheaterArt(false),
			PassengersGainExperience(false),
			ExperienceFromPassengers(true),
			ExperienceFromAirstrike(false),
			AirstrikeExperienceModifier(1.0),
			PassengerExperienceModifier(1.0),
			MindControlExperienceSelfModifier(0.0),
			MindControlExperienceVictimModifier(1.0),
			SpawnExperienceOwnerModifier(0.0),
			SpawnExperienceSpawnModifier(1.0),
			Insignia_ShowEnemy(),
			GattlingCyclic(false),
			IsCustomMissile(false),
			CustomMissileData(),
			CustomMissileWarhead(nullptr),
			CustomMissileEliteWarhead(nullptr),
			CustomMissileTrailerSeparation(3),
			CustomMissileTrailerAnim(nullptr),
			CustomMissileTakeoffAnim(nullptr),
			VoiceRepair(-1),
			HijackerEnterSound(-1),
			HijackerLeaveSound(-1),
			HijackerKillPilots(0),
			HijackerBreakMindControl(true),
			HijackerAllowed(true),
			HijackerOneTime(false),
			WaterImage(nullptr),
			TiberiumProof(),
			TiberiumRemains(),
			TiberiumSpill(false),
			TiberiumTransmogrify(),
			Refinery_UseStorage(false),
			CloakSound(),
			DecloakSound(),
			CloakPowered(false),
			CloakDeployed(false),
			CloakAllowed(true),
			CloakStages(),
			SensorArray_Warn(true),
			CrashSpin(true),
			CanBeReversed(true),
			RadarJamRadius(0),
			PassengerTurret(false),
			AttachedTechnoEffect(OwnerObject),
			Cloneable(true),
			CarryallAllowed(),
			CarryallSizeLimit(),
			EVA_UnitLost(-1),
			ImmuneToAbduction(false),
			OmniCrusher_Aggressive(true),
			ReloadAmount(1),
			FactoryOwners_HaveAllPlans(false)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		bool CameoIsElite(HouseClass const* pHouse) const;

		bool CanBeBuiltAt(BuildingTypeClass const* pFactoryType) const;

		bool CarryallCanLift(UnitClass * Target);

		const char* GetSelectionGroupID() const;

		bool IsGenericPrerequisite() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);

	//static void ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI);
};

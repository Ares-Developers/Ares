#ifndef BUILDINGTYPE_PRISM_FWD_H
#define BUILDINGTYPE_PRISM_FWD_H

#include <BuildingTypeClass.h>
#include "../../Utilities/Template.h"

namespace BuildingTypeExtras {
	class cPrismForwarding {
		public:
		//properties
		enum eEnabled {NO, YES, FORWARD, ATTACK} Enabled;	//is this tower a prism tower? FORWARD means can support, but not attack. ATTACK means can attack but not support.
		DynamicVectorClass<BuildingTypeClass *> Targets;	//the types of buiding that this tower can forward to
		Customizable<signed int> MaxFeeds;					//max number of towers that can feed this tower
		Valueable<signed int> MaxChainLength;				//max length of any given (preceding) branch of the network
		Customizable<signed int> MaxNetworkSize;				//max number of towers that can be in the network
		Customizable<int> SupportModifier; 				//Per-building PrismSupportModifier
		Valueable<signed int> DamageAdd; 					//amount of flat damage to add to the firing beam (before multiplier)
		Customizable<int> MyHeight;						//Per-building PrismSupportHeight
		Valueable<signed int> Intensity;						//amount to adjust beam thickness by when supported
		Valueable<int> ChargeDelay;					//the amount to delay start of charging per backward chain
		Valueable<bool> ToAllies;						//can this tower support allies' towers or not
		Valueable<bool> BreakSupport;					//can the slave tower become a master tower at the last second
		Valueable<signed int> SupportWeaponIndex;
		Valueable<signed int> EliteSupportWeaponIndex;

		//methods
		signed int GetUnusedWeaponSlot(BuildingTypeClass*, bool);
		void Initialize(BuildingTypeClass* );
		void LoadFromINIFile(BuildingTypeClass *, CCINIClass *);

		// future considerations - move these to BuildingExt's PrismForwarding and refactor first arg
		static int AcquireSlaves_MultiStage(BuildingClass *, BuildingClass *, int, int, int *, int *);
		static int AcquireSlaves_SingleStage(BuildingClass *, BuildingClass *, int, int, int *, int *);
		static bool ValidateSupportTower(BuildingClass *, BuildingClass *, BuildingClass *);
		static void SetChargeDelay(BuildingClass *, int);
		static void SetChargeDelay_Get(BuildingClass * , int , int , int , DWORD *, DWORD *);
		static void SetChargeDelay_Set(BuildingClass * , int , DWORD *, DWORD *, int);
		static void RemoveFromNetwork(BuildingClass *, bool);
		static void SetSupportTarget(BuildingClass *, BuildingClass *);
		static void RemoveAllSenders(BuildingClass *);

		// constructor
		cPrismForwarding() : Enabled(NO),
			MaxFeeds(&RulesClass::Instance->PrismSupportMax),
			MaxChainLength(1),
			MaxNetworkSize(&RulesClass::Instance->PrismSupportMax),
			SupportModifier(&RulesClass::Instance->PrismSupportModifier),
			DamageAdd(0),
			MyHeight(&RulesClass::Instance->PrismSupportHeight),
			Intensity(-2),
			ChargeDelay(1),
			ToAllies(false),
			BreakSupport(false),
			SupportWeaponIndex(-1)
		{};
	};
};
#endif

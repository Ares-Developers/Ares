#ifndef SUPERTYPE_EXT_H
#define SUPERTYPE_EXT_H

#include <AircraftTypeClass.h>
#include <AnimClass.h>
#include <CCINIClass.h>
#include <HouseClass.h>
#include <MissionClass.h>
#include <RadarEventClass.h>
#include <SuperClass.h>
#include <SwizzleManagerClass.h>
#include <VocClass.h>
#include <VoxClass.h>

#include "../../Ares.CRT.h"
#include "../../Misc/Actions.h"
//#include "../../Misc/SWTypes.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Template.h"
#include "../../Utilities/Constructs.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

// the index of the first custom sw type
#define FIRST_SW_TYPE 12

#include "../_Container.hpp"

class ParadropPlane;
class NewSWType;

class SWTypeExt
{
public:
	typedef SuperWeaponTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		// SpyPlane
		ValueableIdx<int, AircraftTypeClass> SpyPlane_TypeIndex;
		Valueable<int> SpyPlane_Count;
		ValueableIdx<int, MissionClass> SpyPlane_Mission;

		// Lightning Storm
		Valueable<int> Weather_Duration;
		Valueable<int> Weather_HitDelay;
		Valueable<int> Weather_ScatterDelay;
		Valueable<int> Weather_ScatterCount;
		Valueable<int> Weather_Separation;
		Valueable<int> Weather_CloudHeight;
		Valueable<int> Weather_RadarOutage;
		Valueable<int> Weather_DebrisMin;
		Valueable<int> Weather_DebrisMax;
		Valueable<bool> Weather_PrintText;
		Valueable<bool> Weather_IgnoreLightningRod;
		Valueable<AnimTypeClass*> Weather_BoltExplosion;
		TypeList<AnimTypeClass*> Weather_Clouds;
		TypeList<AnimTypeClass*> Weather_Bolts;
		TypeList<AnimTypeClass*> Weather_Debris;
		TypeList<int> Weather_Sounds;
		ValueableEnum<SuperWeaponAffectedHouse> Weather_RadarOutageAffects;

		// Nuke
		Valueable<WeaponTypeClass*> Nuke_Payload;
		Valueable<AnimTypeClass*> Nuke_PsiWarning;
		Valueable<AnimTypeClass*> Nuke_TakeOff;
		Valueable<bool> Nuke_SiloLaunch;

		// Generic Paradrop
		DynamicVectorClass<ParadropPlane*> *ParaDrop;
		DynamicVectorClass<ParadropPlane*> ParaDropPlanes;

		// Generic Protection
		Customizable<int> Protect_Duration;
		Customizable<int> Protect_PlayFadeSoundTime;
		Customizable<int> Protect_PowerOutageDuration;
		Valueable<bool> Protect_IsForceShield;

		// Chronosphere
		Valueable<AnimTypeClass *> Chronosphere_BlastSrc;
		Valueable<AnimTypeClass *> Chronosphere_BlastDest;
		Valueable<bool> Chronosphere_KillOrganic;
		Valueable<bool> Chronosphere_KillTeleporters;
		Valueable<bool> Chronosphere_AffectUndeployable;
		Valueable<bool> Chronosphere_AffectBuildings;
		Valueable<bool> Chronosphere_AffectUnwarpable;
		Valueable<bool> Chronosphere_AffectIronCurtain;

		// Genetic Mutator
		Valueable<bool> Mutate_Explosion;
		Valueable<bool> Mutate_IgnoreCyborg;
		Valueable<bool> Mutate_IgnoreNotHuman;
		Valueable<bool> Mutate_KillNatural;

		// Psychic Dominator
		Valueable<bool> Dominator_Capture;
		Valueable<int> Dominator_FireAtPercentage;
		Valueable<int> Dominator_FirstAnimHeight;
		Valueable<int> Dominator_SecondAnimHeight;
		Customizable<AnimTypeClass*> Dominator_FirstAnim;
		Customizable<AnimTypeClass*> Dominator_SecondAnim;
		Customizable<AnimTypeClass*> Dominator_ControlAnim;
		Valueable<bool> Dominator_Ripple;
		Valueable<bool> Dominator_CaptureMindControlled;
		Valueable<bool> Dominator_CaptureImmuneToPsionics;
		
		// Sonar
		Valueable<int> Sonar_Delay;

		// Money
		Valueable<int> Money_Amount;
		Valueable<int> Money_DrainAmount;
		Valueable<int> Money_DrainDelay;

		// Generic
		ValueableIdx<int, VoxClass> EVA_Ready;
		ValueableIdx<int, VoxClass> EVA_Activated;
		ValueableIdx<int, VoxClass> EVA_Detected;
		ValueableIdx<int, VoxClass> EVA_Impatient;
		ValueableIdx<int, VoxClass> EVA_InsufficientFunds;

		// anim/sound
		ValueableIdx<int, VocClass> SW_Sound;
		ValueableIdx<int, VocClass> SW_ActivationSound;
		Valueable<AnimTypeClass *> SW_Anim;
		Valueable<int> SW_AnimHeight;
		ValueableEnum<SuperWeaponAffectedHouse> SW_AnimVisibility;

		Valueable<bool> SW_TypeCustom;
		Valueable<bool> SW_AutoFire;
		Valueable<bool> SW_ManualFire;
		Valueable<bool> SW_FireToShroud;
		Valueable<bool> SW_RadarEvent;
		Valueable<bool> SW_ShowCameo;
		Valueable<bool> SW_Unstoppable;
		Valueable<MouseCursor> SW_Cursor;
		Valueable<MouseCursor> SW_NoCursor;
		char SW_PostDependent[0x18];
		ValueableEnum<SuperWeaponAITargetingMode> SW_AITargetingType;
		Customizable<double> SW_ChargeToDrainRatio;

		Valueable<float> SW_WidthOrRange;
		Valueable<int> SW_Height;
		ValueableEnum<SuperWeaponAffectedHouse> SW_AffectsHouse;
		ValueableEnum<SuperWeaponTarget> SW_AffectsTarget;
		ValueableEnum<SuperWeaponTarget> SW_RequiresTarget;
		Customizable<WarheadTypeClass *> SW_Warhead;
		Valueable<int> SW_Damage;
		Valueable<int> SW_Deferment;

		// Lighting
		Valueable<bool> Lighting_Enabled;
		Customizable<int> Lighting_Ambient;
		Customizable<int> Lighting_Green;
		Customizable<int> Lighting_Blue;
		Customizable<int> Lighting_Red;

		// Messages
		char Message_Launch[0x20];
		char Message_Activate[0x20];
		char Message_Abort[0x20];
		char Message_InsufficientFunds[0x20];
		Valueable<int> Message_ColorScheme;
		Valueable<bool> Message_FirerColor;

		// Texts
		char Text_Preparing[0x20];
		char Text_Hold[0x20];
		char Text_Ready[0x20];
		char Text_Charging[0x20];
		char Text_Active[0x20];

		CustomPalette CameoPal;

		// Unit Delivery
		DynamicVectorClass<TechnoTypeClass *> SW_Deliverables;
		Valueable<bool> SW_DeliverBuildups;

		char SidebarPCX[0x20];

		int HandledByNewSWType;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			SpyPlane_TypeIndex (0),
			SpyPlane_Count (1),
			SpyPlane_Mission (mission_AttackAgain),
			Weather_CloudHeight (-1),
			Weather_ScatterCount (1),
			Nuke_PsiWarning (NULL),
			Sonar_Delay (0),
			SW_ActivationSound (-1),
			SW_ChargeToDrainRatio (&RulesClass::Instance->ChargeToDrainRatio),
			Money_Amount (0),
			Money_DrainAmount (0),
			Money_DrainDelay (0),
			EVA_Ready (-1),
			EVA_Activated (-1),
			EVA_Detected (-1),
			EVA_Impatient (-1),
			EVA_InsufficientFunds (-1),
			Message_ColorScheme (-1),
			Message_FirerColor (false),
			Lighting_Enabled (true),
			SW_Sound (-1),
			SW_Anim (NULL),
			SW_AnimHeight (0),
			SW_AnimVisibility (0),
			SW_TypeCustom (false),
			SW_AutoFire (false),
			SW_ManualFire (true),
			SW_ShowCameo (true),
			SW_Unstoppable (false),
			SW_AffectsHouse (SuperWeaponAffectedHouse::All),
			SW_AffectsTarget (SuperWeaponTarget::All),
			SW_RequiresTarget (SuperWeaponTarget::None),
			SW_AITargetingType (SuperWeaponAITargetingMode::None),
			SW_FireToShroud (true),
			SW_RadarEvent (false),
			SW_WidthOrRange (-1),
			SW_Height (-1),
			HandledByNewSWType (-1),
			CameoPal(),
			SW_DeliverBuildups (false),
			SW_Damage(0)
			{
				*SidebarPCX = 0;
				*SW_PostDependent = 0;
				*Message_Launch = 0;
				*Message_Activate = 0;
				*Message_Abort = 0;
				*Message_InsufficientFunds = 0;
				*Text_Preparing = 0;
				*Text_Ready = 0;
				*Text_Hold = 0;
				*Text_Charging = 0;
				*Text_Active = 0;
			};

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void InitializeRuled(TT *pThis);

		bool ChangeLighting();
		bool IsAnimVisible(HouseClass* pFirer);
		bool CanFireAt(CellStruct *pCoords);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse);
		bool IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, SuperWeaponAffectedHouse::Value value);
		bool IsTechnoAffected(TechnoClass* pTechno);
		void PrintMessage(char* Message, HouseClass* pFirer);
		NewSWType* GetNewSWType();

		virtual void InvalidatePointer(void *ptr) {
		}

	private:
		static SuperWeaponAffectedHouse::Value GetRelation(HouseClass* pFirer, HouseClass* pHouse);
		bool IsCellEligible(CellClass* pCell, SuperWeaponTarget::Value allowed);
		bool IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget::Value allowed);
	};

	static Container<SWTypeExt> ExtMap;

	static SuperWeaponTypeClass *CurrentSWType;

	bool static Launch(SuperClass* pThis, NewSWType* pData, CellStruct* pCoords, byte IsPlayer);
	bool static __stdcall SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
	void static ClearChronoAnim(SuperClass *pThis);
	void static CreateChronoAnim(SuperClass *pThis, CoordStruct *pCoords, AnimTypeClass *pAnimType);
	bool static ChangeLighting(SuperClass *pThis);
	bool static ChangeLighting(SuperWeaponTypeClass *pThis);
};

class ParadropPlane {
public:
	AircraftTypeClass *pAircraft;
	TypeList<TechnoTypeClass*> pTypes;
	TypeList<int> pNum;

	ParadropPlane() : pAircraft (NULL)
	{
	}
};

#endif

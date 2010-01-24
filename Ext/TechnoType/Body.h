#ifndef TECHNOTYPE_EXT_H
#define TECHNOTYPE_EXT_H

#include <TechnoTypeClass.h>

#include "../../Ares.h"
#include "../_Container.hpp"
#include "../../Utilities/Template.h"
#include "../../Utilities/Constructs.h"

class TechnoTypeExt
{
public:
	typedef TechnoTypeClass TT;
	enum SpotlightAttachment { sa_Body, sa_Turret, sa_Barrel };

	class ExtData : public Extension<TT>
	{
	public:
		DynamicVectorClass<InfantryTypeClass *> Survivors_Pilots;
		Promotable<int> Survivors_PilotChance;
		Promotable<int> Survivors_PassengerChance;
		// new on 28.09.09 for #631
		int Survivors_PilotCount; //!< Defines the number of pilots inside this vehicle if Crewed=yes; maximum number of pilots who can survive. Defaults to 0 if Crewed=no; defaults to 1 if Crewed=yes. // NOTE: Flag in INI is called Survivor.Pilots

		// animated cameos
//		int Cameo_Interval;
//		int Cameo_CurrentFrame;
//		TimerStruct Cameo_Timer;

		DynamicVectorClass< DynamicVectorClass<int>* > PrerequisiteLists;
		DynamicVectorClass<int> PrerequisiteNegatives;
		DWORD PrerequisiteTheaters;

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

		bool WaterAlt;

		// these are not implemented at all yet
		DynamicVectorClass<WeaponStruct> Weapons;
		DynamicVectorClass<WeaponStruct> EliteWeapons;

		Promotable<SHPStruct *> Insignia;

		Customizable<AnimTypeClass*> Parachute_Anim;

		// new on 08.11.09 for #342 (Operator=)
		InfantryTypeClass * Operator; //!< Saves a pointer to an InfantryType required to be a passenger of this unit in order for it to work. Defaults to NULL. \sa TechnoClass_Update_CheckOperators, bool IsAPromiscuousWhoreAndLetsAnyoneRideIt
		bool IsAPromiscuousWhoreAndLetsAnyoneRideIt; //!< If this is true, Operator= is not checked, and the object will work with any passenger, provided there is one. \sa InfantryTypeClass * Operator

		CustomPalette CameoPal;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			Survivors_PilotChance (NULL),
			Survivors_PassengerChance (NULL),
			Survivors_PilotCount (0),
			PrerequisiteTheaters (0xFFFFFFFF),
			Secret_RequiredHouses (0),
			Secret_ForbiddenHouses (0),
			Is_Deso (false),
			Is_Deso_Radiation (false),
			Is_Cow (false),
			Is_Spotlighted (false),
			Spot_Height (200),
			Spot_Distance (1024),
			Spot_AttachedTo (sa_Body),
			Spot_DisableR (false),
			Spot_DisableG (false),
			Spot_DisableB (false),
			Spot_Reverse (false),
			Is_Bomb (false),
			WaterAlt (false),
			Insignia (NULL),
			Parachute_Anim(&RulesClass::Instance->Parachute),
			Operator (NULL),
			IsAPromiscuousWhoreAndLetsAnyoneRideIt (false),
			CameoPal()
			{ this->Insignia.SetAll(NULL); };

		virtual ~ExtData() {};

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr) {
			AnnounceInvalidPointer(Operator, ptr);
		}
};

	static Container<TechnoTypeExt> ExtMap;

	static void PointerGotInvalid(void *ptr);

//	static void ReadWeapon(WeaponStruct *pWeapon, const char *prefix, const char *section, CCINIClass *pINI);
};

#endif

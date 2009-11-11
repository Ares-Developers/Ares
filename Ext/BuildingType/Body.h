#ifndef BUILDINGTYPE_EXT_H
#define BUILDINGTYPE_EXT_H

#include <CCINIClass.h>
#include <BuildingTypeClass.h>
#include <InfantryTypeClass.h>
#include <Randomizer.h>
#include <UnitTypeClass.h>

#include <Helpers\Macro.h>

#include "..\_Container.hpp"
#include "..\..\Ares.h"

//ifdef DEBUGBUILD -- legit needs to log things, so no debug
#include "..\..\Misc\Debug.h"
//endif

class BuildingClass;

#define FOUNDATION_CUSTOM	0x7F

class BuildingTypeExt
{
public:
	typedef BuildingTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		// solid
		int Solid_Height;

		// foundations
		bool IsCustom;
		int CustomWidth;
		int CustomHeight;
		int OutlineLength;
		CellStruct* CustomData;
		CellStruct* OutlineData;

		// new secret lab
		DynamicVectorClass<TechnoTypeClass *> Secret_Boons;
		bool Secret_RecalcOnCapture;
		bool Secret_Placed;

		// new firestorm wall
		bool Firewall_Is;

		// added on 11.11.09 for #221 and children (Trenches)
		double UCPassThrough; 					//! how many percent of the shots pass through the building to the occupants
		double UCFatalRate; 					//! chance of someone getting shot in the head, basically
		double UCDamageMultiplier; 				//! how many percent of normal damage are applied if an occupant is hit
		bool BunkerRaidable; 					//! can this BuildingType be occupied by hostile forces despite being owned by a player, if empty?
		signed int IsTrench; 					//! enables moving between segments - saves ID of a kind of trench
		BuildingTypeClass * RubbleIntact; 		//! What BuildingType to turn into when repaired
		BuildingTypeClass * RubbleDestroyed;	//! What BuildingType to turn into when destroyed

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
			Solid_Height (0),
			IsCustom (false),
			CustomData (NULL),
			OutlineData (NULL),
			CustomWidth (0),
			CustomHeight (0),
			Firewall_Is (false),
			UCPassThrough (0.0),
			UCFatalRate (0.0),
			UCDamageMultiplier (1.0),
			BunkerRaidable (false),
			IsTrench (-1),
			RubbleIntact (NULL),
			RubbleDestroyed (NULL)
			{ };

		virtual ~ExtData() {
			delete CustomData;
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
	};

	static Container<BuildingTypeExt> ExtMap;
//	static ExtData ExtMap;

	static void UpdateSecretLabOptions(BuildingClass *pThis);
};

#endif

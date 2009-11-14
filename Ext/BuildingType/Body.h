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

#include <vector>

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
		double UCPassThrough; 					//!< How many percent of the shots pass through the building to the occupants? 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		double UCFatalRate; 					//!< Chance of an occupant getting killed instantly when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 0.0.
		double UCDamageMultiplier; 				//!< How many percent of normal damage are applied if an occupant is hit when a bullet passes through. 0.0 = 0%, 1.0 = 100%; Defaults to 1.0.
		bool BunkerRaidable; 					//!< Can this BuildingType be occupied by hostile forces despite being owned by a player, if empty?
		signed int IsTrench; 					//!< Enables moving between segments - saves ID of a kind of trench. \sa trenchKinds
		BuildingTypeClass * RubbleIntact; 		//!< What BuildingType to turn into when repaired (rubble repair, not normal repair).
		BuildingTypeClass * RubbleDestroyed;	//!< What BuildingType to turn into when destroyed (rubble destroyed, not normal destroyed).
		static std::vector<std::string> trenchKinds; //!< Vector of strings associating known trench names with IsTrench IDs. \sa IsTrench

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

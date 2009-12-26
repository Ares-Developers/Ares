#ifndef BUILDING_EXT_H
#define BUILDING_EXT_H

#include <CCINIClass.h>
#include <BuildingClass.h>

#include <Helpers/Macro.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

class BuildingExt
{
public:
	typedef BuildingClass TT;

	class ExtData : public Extension<TT>
	{
	private:
		BuildingClass* RubbleState; //!< This BuildingClass will be used as the building's rubble state after the building was destroyed. Its type should have \link BuildingTypeExt::ExtData::RubbleIntact Rubble.Intact \endlink set.
		BuildingClass* NormalState; //!< This BuildingClass will be used as the building's normal state after the building was reconstructed from rubble. Its type should have \link BuildingTypeExt::ExtData::RubbleDestroyed Rubble.Destroyed \endlink set.

	public:
		HouseClass* OwnerBeforeRaid; //!< Contains the house which owned this building prior to it being raided and turned over to the raiding party.
		bool isCurrentlyRaided; //!< Whether this building is currently occupied by someone not the actual owner of the structure.

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension(Canary, OwnerObject),
			RubbleState(NULL), NormalState(NULL),
			OwnerBeforeRaid(NULL), isCurrentlyRaided(false)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		// related to Advanced Rubble
		void RubbleYell(bool beingRepaired = false); // This function triggers back and forth between rubble states.
		void setRubble(BuildingClass* arg) {RubbleState = arg;} //!< Public setter for #RubbleState, used to set a link back on NormalState from RubbleState. \param arg the building to use as rubble. \sa RubbleYell()
		void setNormal(BuildingClass* arg) {NormalState = arg;} //!< Public setter for #NormalState, used to set a link back on RubbleState from NormalState. \param arg the normal building. \sa RubbleYell()

		// related to trench traversal
		bool canTraverseTo(BuildingClass* targetBuilding); // Returns true if people can move from the current building to the target building, otherwise false.
		void doTraverseTo(BuildingClass* targetBuilding); // This function moves as many occupants as possible from the current to the target building.
		bool sameTrench(BuildingClass* targetBuilding); // Checks if both buildings are of the same trench kind.

		// related to linkable buildings
		bool isLinkable(); //!< Returns true if this is a structure that can be linked to other structures, like a wall, fence, or trench. This is used to quick-check if the game has to look for linkable buildings in the first place. \sa canLinkTo()
		bool canLinkTo(BuildingClass* targetBuilding); //!< Checks if the current building can link to the given target building. \param targetBuilding the building to check for compatibility. \return true if the two buildings can be linked. \sa isLinkable()
	};

	static Container<BuildingExt> ExtMap;

	static DWORD GetFirewallFlags(BuildingClass *pThis);

	//static void ExtendFirewall(BuildingClass *pThis, CellStruct Center, HouseClass *Owner); // replaced by generic buildLines
	static void buildLines(BuildingClass*, CellStruct, HouseClass*); // Does the actual line-building, using isLinkable() and canLinkTo().

	static void UpdateDisplayTo(BuildingClass *pThis);

	static signed int GetImageFrameIndex(BuildingClass *pThis);
};

#endif

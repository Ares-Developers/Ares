#ifndef JAMMER_H
#define JAMMER_H

#include <vector>
#include <TechnoClass.h>
#include <BuildingClass.h>
// yay circular dependencies...let's see if this works out :S
#include "../Ext/Techno/Body.h"

class JammerClass {
  private:
	static const int ScanInterval = 30; 	//!< Minimum delay between scans in frames (two seconds/thirty frames).
	int LastScan;							//!< Frame number when the last scan was performed.

	TechnoClass* AttachedToObject;			//!< Pointer to game object this jammer is on
	TechnoExt::ExtData* Ext; 				//!< Pointer to the ExtData object this jammer is in

	bool InRangeOf(BuildingClass *);		//!< Calculates if the jammer is in range of this building.
	bool IsEligible(BuildingClass *);		//!< Checks if this building can/should be jammed.

	void Jam(BuildingClass *);				//!< Attempts to jam the given building. (Actually just registers the Jammer with it, the jamming happens in a hook.)
	void Unjam(BuildingClass *);			//!< Attempts to unjam the given building. (Actually just unregisters the Jammer with it, the unjamming happens in a hook.)

  public:
	JammerClass(TechnoClass* GameObject) : LastScan(0), AttachedToObject(NULL), Ext(NULL) {
		this->AttachedToObject = GameObject;
		this->Ext = TechnoExt::ExtMap.Find(GameObject);
	}

	JammerClass(TechnoClass* GameObject, TechnoExt::ExtData* pExt) : LastScan(0), AttachedToObject(NULL), Ext(NULL) {
		this->AttachedToObject = GameObject;
		this->Ext = pExt;
	}

	void UnjamAll();						//!< Unregisters this Jammer on all structures.
	void Update();							//!< Updates this Jammer's status on all eligible structures.
};
#endif

#pragma once

#include "../Misc/Savegame.h"

class TechnoClass;
class BuildingClass;

class JammerClass {
  private:
	static const int ScanInterval = 30; 	//!< Minimum delay between scans in frames (two seconds/thirty frames).
	int LastScan;							//!< Frame number when the last scan was performed.

	TechnoClass* AttachedToObject;			//!< Pointer to game object this jammer is on

	bool Registered;						//!< Did I jam anything at all? Used to skip endless unjam calls.

	bool InRangeOf(BuildingClass *);		//!< Calculates if the jammer is in range of this building.
	bool IsEligible(BuildingClass *);		//!< Checks if this building can/should be jammed.

	void Jam(BuildingClass *);				//!< Attempts to jam the given building. (Actually just registers the Jammer with it, the jamming happens in a hook.)
	void Unjam(BuildingClass *);			//!< Attempts to unjam the given building. (Actually just unregisters the Jammer with it, the unjamming happens in a hook.)

  public:
	JammerClass(TechnoClass* GameObject) :
		LastScan(0),
		AttachedToObject(GameObject),
		Registered(false)
	{ }

	~JammerClass() {
		this->UnjamAll();
	}

	void UnjamAll();						//!< Unregisters this Jammer on all structures.
	void Update();							//!< Updates this Jammer's status on all eligible structures.

	bool Load(AresStreamReader &Stm, bool RegisterForChange);
	bool Save(AresStreamWriter &Stm) const;
};

template <>
struct Savegame::ObjectFactory<JammerClass> {
	std::unique_ptr<JammerClass> operator() (AresStreamReader &Stm) const {
		return std::make_unique<JammerClass>(nullptr);
	}
};

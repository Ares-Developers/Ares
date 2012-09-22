#ifndef __POWEREDUNITCLASS_H
#define __POWEREDUNITCLASS_H
#include <TechnoClass.h>
#include <BuildingTypeClass.h>
#include "../Ext/Techno/Body.h"

class PoweredUnitClass
{
private:
	static const int ScanInterval = 15;		//!< Minimum delay between scans in frames.
	
	TechnoClass* Techno;
	TechnoTypeExt::ExtData* Ext;
	int LastScan;							//!< Frame number when the last scan was performed.
	bool Powered;							//!< Whether the unit has a building providing power. NOT the same as being online.
	
	bool IsPoweredBy(HouseClass* Owner) const;
	void PowerUp();
	bool PowerDown();
public:
	PoweredUnitClass(TechnoClass* Techno, TechnoTypeExt::ExtData* Ext)
		: Techno(Techno), Ext(Ext), LastScan(0), Powered(true) {
	}
	
	~PoweredUnitClass() {
	}

	//!< Updates this Powered Unit's status. Returns whether the unit should stay alive.
	bool Update();
	
	//!< Whether the unit has a building providing power. NOT the same as being online.
	inline bool IsPowered() const {
		return this->Powered;
	}
};

#endif

#ifndef RMG_EXT_H
#define RMG_EXT_H

#include <ArrayClasses.h>
#include <StringTable.h>
#include <CCINIClass.h>
#include <UnitTypeClass.h>
#include <InfantryTypeClass.h>
#include <BuildingTypeClass.h>
#include "Debug.h"
#include "../Utilities/Constructs.h"

class RMG {
public:
	static bool UrbanAreas;
	static bool UrbanAreasRead;
	static int UrbanStructuresReadSoFar;
	static VectorNames<BuildingTypeClass> UrbanStructures;
	static VectorNames<UnitTypeClass> UrbanVehicles;
	static VectorNames<InfantryTypeClass> UrbanInfantry;
};

#endif

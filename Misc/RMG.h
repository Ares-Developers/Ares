#ifndef RMG_EXT_H
#define RMG_EXT_H

#include <ArrayClasses.h>
#include <StringTable.h>
#include <CCINIClass.h>
#include "Debug.h"

class RMG {
public:
	static bool UrbanAreas;
	static bool UrbanAreasRead;
	static DynamicVectorClass<char*> UrbanStructures;
	static int UrbanStructuresReadSoFar;
	static DynamicVectorClass<char*> UrbanVehicles;
	static DynamicVectorClass<char*> UrbanInfantry;

	static long startTime;
};

#endif

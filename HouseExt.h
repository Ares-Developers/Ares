#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include <hash_map>
#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <FactoryClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <SuperClass.h>

class HouseClassExt
{
	public:
	struct HouseClassData
	{
		// Generic
		bool Is_Initialized;

		void Initialize(HouseClass* pThis);
	};

	EXT_P_DECLARE(HouseClass);
	EXT_FUNCS(HouseClass);

	static signed int BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem);
	static signed int CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued);

	static signed int RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem);

	static signed int PrereqValidate
		(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued);
};

#endif

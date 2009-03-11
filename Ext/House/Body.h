#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include "..\_Container.hpp"
#include <MacroHelpers.h> //basically indicates that this is DCoder country

#include <FactoryClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <SuperClass.h>

class HouseExt
{
	public:
	typedef HouseClass TT;
	
	class ExtData : public Extension<TT> 
	{
		public:
			bool IonSensitive;
		ExtData(const DWORD Canary = 0) : 
			IonSensitive(0)
		{
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };
	};

	static Container<HouseExt> ExtMap;

	static signed int BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem);
	static signed int CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued);

	static signed int RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem);
	static bool PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem);

	static signed int PrereqValidate
		(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued);
};

#endif

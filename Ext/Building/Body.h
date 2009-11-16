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
	public:
		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };
	};

	static Container<BuildingExt> ExtMap;

	static DWORD GetFirewallFlags(BuildingClass *pThis);

	static void ExtendFirewall(BuildingClass *pThis, CellStruct Center, HouseClass *Owner);

	static void UpdateDisplayTo(BuildingClass *pThis);

};

#endif

#include <YRPP.h>
#include "HouseExt.h"

EXT_P_DEFINE(HouseClass);

EXT_CTOR(HouseClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_SAVE(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXPORT_FUNC(HouseClass_PrereqValidator)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */
	return 0x4F8361;
}

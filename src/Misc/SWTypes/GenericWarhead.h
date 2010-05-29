#ifndef SUPERTYPE_EXT_GWARH_H
#define SUPERTYPE_EXT_GWARH_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_GenericWarhead : NewSWType
{
	public:
		SW_GenericWarhead() : NewSWType()
			{ };

		virtual ~SW_GenericWarhead()
			{ };

		virtual const char * GetTypeString()
			{ return "GenericWarhead"; }

	virtual void LoadFromINI(
		SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	//virtual bool CanFireAt(CellStruct *pCoords); // we'll be using NewSWType's
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
};
#endif

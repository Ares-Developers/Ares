#ifndef SUPERTYPE_EXT_GWARH_H
#define SUPERTYPE_EXT_GWARH_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_GenericWarhead : public NewSWType
{
	public:
		SW_GenericWarhead() : NewSWType()
			{ };

		virtual ~SW_GenericWarhead()
			{ };

		virtual const char * GetTypeString()
			{ return "GenericWarhead"; }

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
};
#endif

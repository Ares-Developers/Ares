#ifndef SUPERTYPE_EXT_PARADROP_H
#define SUPERTYPE_EXT_PARADROP_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_ParaDrop : NewSWType
{
	public:
		SW_ParaDrop() : NewSWType()
			{ };

		virtual ~SW_ParaDrop()
			{ };

		virtual const char * GetTypeString()
			{ return NULL; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
		
		bool SendParadrop(SuperClass* pThis, CellClass* pCell);
};
#endif

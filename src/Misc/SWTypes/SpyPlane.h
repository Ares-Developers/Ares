#ifndef SUPERTYPE_EXT_SPYPLANE_H
#define SUPERTYPE_EXT_SPYPLANE_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_SpyPlane : NewSWType
{
	public:
		SW_SpyPlane() : NewSWType()
			{ };

		virtual ~SW_SpyPlane()
			{ };

		virtual const char * GetTypeString()
			{ return NULL; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
};
#endif

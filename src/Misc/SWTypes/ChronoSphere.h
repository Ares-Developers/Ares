#ifndef SUPERTYPE_EXT_CHRONOSPHERE_H
#define SUPERTYPE_EXT_CHRONOSPHERE_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_ChronoSphere : NewSWType
{
	public:
		SW_ChronoSphere() : NewSWType()
			{ };

		virtual ~SW_ChronoSphere()
			{ };

		virtual const char * GetTypeString()
			{ return NULL; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();
};
#endif

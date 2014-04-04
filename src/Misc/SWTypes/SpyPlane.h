#ifndef SUPERTYPE_EXT_SPYPLANE_H
#define SUPERTYPE_EXT_SPYPLANE_H

#include "../SWTypes.h"

class SW_SpyPlane : public NewSWType
{
	public:
		SW_SpyPlane() : NewSWType()
			{ };

		virtual ~SW_SpyPlane()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
};
#endif

#ifndef SUPERTYPE_EXT_SPYPLANE_H
#define SUPERTYPE_EXT_SPYPLANE_H

#include "../SWTypes.h"

class SW_SpyPlane : public NewSWType
{
public:
	SW_SpyPlane() : NewSWType()
		{ };

	virtual ~SW_SpyPlane() override
		{ };

	virtual const char * GetTypeString() override
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
};

#endif

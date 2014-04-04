#ifndef SUPERTYPE_EXT_PARADROP_H
#define SUPERTYPE_EXT_PARADROP_H

#include "../SWTypes.h"

class SW_ParaDrop : public NewSWType
{
public:
	SW_ParaDrop() : NewSWType()
		{ };

	virtual ~SW_ParaDrop() override
		{ };

	virtual const char * GetTypeString() override
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
		
	bool SendParadrop(SuperClass* pThis, CellClass* pCell);
};

#endif

#pragma once

#include "../SWTypes.h"

class SW_DropPod : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "DropPod";
	}

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
};

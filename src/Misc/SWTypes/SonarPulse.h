#pragma once

#include "../SWTypes.h"

class SW_SonarPulse : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "SonarPulse";
	}

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual SuperWeaponFlags Flags() const override;

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

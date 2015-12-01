#pragma once

#include "../SWTypes.h"

class SW_HunterSeeker : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "HunterSeeker";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;

	CellStruct GetLaunchCell(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const;
};

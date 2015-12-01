#pragma once

#include "../SWTypes.h"

class SW_Protect : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "Protect";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool CanFireAt(TargetingData const& data, const CellStruct& cell, bool manual) const override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(SuperWeaponType type) const override;

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

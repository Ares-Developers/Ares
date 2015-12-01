#pragma once

#include "../SWTypes.h"

class SW_EMPulse : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "EMPulse";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;

private:
	virtual bool IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const override;
	virtual std::pair<double, double> GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const override;
};

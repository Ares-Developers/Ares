#pragma once

#include "../SWTypes.h"

class SW_NuclearMissile : public NewSWType
{
public:
	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags() const override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;

	static SuperWeaponTypeClass* CurrentNukeType;

private:
	virtual bool IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const override;
};

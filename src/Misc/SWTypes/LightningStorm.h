#ifndef SUPERTYPE_EXT_LSTORM_H
#define SUPERTYPE_EXT_LSTORM_H

#include "../SWTypes.h"

class SW_LightningStorm : public NewSWType
{
public:
	SW_LightningStorm() : NewSWType()
		{ };

	virtual ~SW_LightningStorm() override
		{ };

	virtual const char * GetTypeString() override
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
	virtual SuperWeaponFlags::Value Flags() override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;

	static SuperClass* CurrentLightningStorm;
};

#endif

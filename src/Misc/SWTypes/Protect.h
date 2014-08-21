#ifndef SUPERTYPE_EXT_PROTECT_H
#define SUPERTYPE_EXT_PROTECT_H

#include "../SWTypes.h"

class SW_Protect : public NewSWType
{
public:
	SW_Protect() : NewSWType()
		{ };

	virtual const char* GetTypeString() const override
	{
		return "Protect";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) const override;

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

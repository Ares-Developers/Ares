#ifndef SUPERTYPE_EXT_CHRONOSPHERE_H
#define SUPERTYPE_EXT_CHRONOSPHERE_H

#include "../SWTypes.h"

class SW_ChronoSphere : public NewSWType
{
public:
	SW_ChronoSphere() : NewSWType()
		{ };

	virtual ~SW_ChronoSphere() override
		{ };

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
	virtual SuperWeaponFlags::Value Flags() override;

	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

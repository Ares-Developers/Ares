#ifndef SUPERTYPE_EXT_REVEAL_H
#define SUPERTYPE_EXT_REVEAL_H

#include "../SWTypes.h"

class SW_Reveal : public NewSWType
{
public:
	SW_Reveal() : NewSWType()
		{ };

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) const override;

	virtual int GetSound(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

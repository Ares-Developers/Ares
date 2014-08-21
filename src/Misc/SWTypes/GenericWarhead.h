#ifndef SUPERTYPE_EXT_GWARH_H
#define SUPERTYPE_EXT_GWARH_H

#include "../SWTypes.h"

class SW_GenericWarhead : public NewSWType
{
public:
	SW_GenericWarhead() : NewSWType()
		{ };

	virtual const char* GetTypeString() const override
	{
		return "GenericWarhead";
	}

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
};

#endif

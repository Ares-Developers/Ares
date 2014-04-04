#ifndef SUPERTYPE_EXT_PARADROP_H
#define SUPERTYPE_EXT_PARADROP_H

#include "../SWTypes.h"

class SW_ParaDrop : public NewSWType
{
public:
	SW_ParaDrop() : NewSWType()
		{ };

	virtual ~SW_ParaDrop()
		{ };

	virtual const char * GetTypeString()
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
	virtual bool HandlesType(int type);
		
	bool SendParadrop(SuperClass* pThis, CellClass* pCell);
};

#endif

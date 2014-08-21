#ifndef SUPERTYPE_EXT_PARADROP_H
#define SUPERTYPE_EXT_PARADROP_H

#include "../SWTypes.h"

class SW_ParaDrop : public NewSWType
{
public:
	SW_ParaDrop() : NewSWType()
		{ };

	virtual ~SW_ParaDrop() override
		{ };

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
		
	bool SendParadrop(SuperClass* pThis, CellClass* pCell);

	static void SendPDPlane(HouseClass* pOwner, CellClass* pDestination,
		AircraftTypeClass* pPlaneType, const Iterator<TechnoTypeClass*> &Types,
		const Iterator<int> &Nums);
};

#endif

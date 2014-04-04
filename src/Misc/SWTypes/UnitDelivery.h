#ifndef SUPERTYPE_EXT_UNITDELIVERY_H
#define SUPERTYPE_EXT_UNITDELIVERY_H

#include "../SWTypes.h"

class SW_UnitDelivery : public NewSWType
{
public:
	SW_UnitDelivery() : NewSWType()
		{ };

	virtual ~SW_UnitDelivery()
		{ };

	virtual const char * GetTypeString()
		{ return "UnitDelivery"; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);

	typedef UnitDeliveryStateMachine TStateMachine;

	void newStateMachine(int Duration, CellStruct XY, SuperClass *pSuper) {
		SWStateMachine::Register(std::make_unique<UnitDeliveryStateMachine>(Duration, XY, pSuper, this));
	}
};

#endif

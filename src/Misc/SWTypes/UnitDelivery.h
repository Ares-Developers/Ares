#ifndef SUPERTYPE_EXT_UNITDELIVERY_H
#define SUPERTYPE_EXT_UNITDELIVERY_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_UnitDelivery : NewSWType
{
	public:
		SW_UnitDelivery() : NewSWType()
			{ };

		virtual ~SW_UnitDelivery()
			{ };

		virtual const char * GetTypeString()
			{ return "UnitDelivery"; }

	virtual void LoadFromINI(
		SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);

	typedef UnitDeliveryStateMachine TStateMachine;

	void newStateMachine(int Duration, CellStruct XY, SuperClass *pSuper) {
		new TStateMachine(Duration, XY, pSuper, this);
	}
};
#endif

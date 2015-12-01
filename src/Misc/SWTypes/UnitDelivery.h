#pragma once

#include "../SWTypes.h"

class SW_UnitDelivery : public NewSWType
{
public:
	virtual const char* GetTypeString() const override
	{
		return "UnitDelivery";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;

	using TStateMachine = UnitDeliveryStateMachine;

	void newStateMachine(int Duration, CellStruct XY, SuperClass *pSuper) {
		SWStateMachine::Register(std::make_unique<UnitDeliveryStateMachine>(Duration, XY, pSuper, this));
	}
};

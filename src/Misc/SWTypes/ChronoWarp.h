#pragma once

#include "../SWTypes.h"

class SW_ChronoWarp : public NewSWType
{
public:
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(SuperWeaponType type) const override;
	virtual SuperWeaponFlags Flags() const override;

	using TStateMachine = ChronoWarpStateMachine;

	void newStateMachine(int Duration, const CellStruct &XY, SuperClass* pSuper, NewSWType* pSWType,
		DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> Buildings) {
			SWStateMachine::Register(std::make_unique<ChronoWarpStateMachine>(Duration, XY, pSuper, this, std::move(Buildings)));
	}
};

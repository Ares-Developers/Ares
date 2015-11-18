#pragma once

#include "../SWTypes.h"

class SW_Firewall : public NewSWType {
public:
	virtual ~SW_Firewall() override {
		SW_Firewall::FirewallType = SuperWeaponType::Invalid;
	}

	virtual const char* GetTypeString() const override
	{
		return "Firestorm";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override {
		pSW->Action = Action::None;
		pSW->UseChargeDrain = true;
		pData->SW_RadarEvent = false;
		// what can we possibly configure here... warhead/damage inflicted? anims?
	};

	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;

	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) override;

	static SuperWeaponType FirewallType;

protected:
	virtual void SetTypeIndex(int const index) override
	{
		NewSWType::SetTypeIndex(index);

		auto const idxType = SWTypeExt::FirstCustomType + this->GetTypeIndex();
		SW_Firewall::FirewallType = static_cast<SuperWeaponType>(idxType);
	}
};

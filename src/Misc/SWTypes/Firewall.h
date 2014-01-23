#ifndef SUPERTYPE_EXT_FIREWALL_H_
#define SUPERTYPE_EXT_FIREWALL_H_

#include <xcompile.h>
#include "../SWTypes.h"

class SW_Firewall : NewSWType {
public:
	SW_Firewall() : NewSWType() {
		SW_Firewall::FirewallTypeIndex = FIRST_SW_TYPE + this->TypeIndex;
	};

	virtual ~SW_Firewall() {
		SW_Firewall::FirewallTypeIndex = -1;
	};

	virtual const char * GetTypeString()
	{
		return "Firestorm";
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) {
		pSW->Action = 0;
		pSW->UseChargeDrain = true;
		pData->SW_RadarEvent = false;
		// what can we possibly configure here... warhead/damage inflicted? anims?
	};
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);

	static int FirewallTypeIndex;
};

#endif

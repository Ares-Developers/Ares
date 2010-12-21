#ifndef SUPERTYPE_EXT_FIREWALL_H_
#define SUPERTYPE_EXT_FIREWALL_H_

#include <xcompile.h>
#include "../SWTypes.h"

class SW_Firewall : NewSWType {
	public:
		SW_Firewall() : NewSWType()
			{ };

		virtual ~SW_Firewall()
			{ };

		virtual const char * GetTypeString()
			{ return "Firestorm"; }

		virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) {
			pSW->Action = 0;
			pSW->UseChargeDrain = true;
			pData->SW_RadarEvent = false;
			// what can we possibly configure here... warhead/damage inflicted? anims?
		};
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
};

#endif

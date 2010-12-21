#ifndef SUPERTYPE_EXT_SONAR_H
#define SUPERTYPE_EXT_SONAR_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_SonarPulse : NewSWType
{
	public:
		SW_SonarPulse() : NewSWType()
			{ };

		virtual ~SW_SonarPulse()
			{ };

		virtual const char * GetTypeString()
			{ return "SonarPulse"; }

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
	virtual void LoadFromINI(
		SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
	virtual SuperWeaponFlags::Value Flags();
};
#endif

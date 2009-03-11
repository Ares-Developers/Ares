#ifndef SUPERTYPE_EXT_SONAR_H
#define SUPERTYPE_EXT_SONAR_H

#include <hash_map>

#include "..\SWTypes.h"

class SW_SonarPulse : NewSWType
{
	public:
		SW_SonarPulse() : NewSWType()
			{ };

		virtual ~SW_SonarPulse()
			{ };

		virtual const char * GetTypeString()
			{ return "SonarPulse"; }

	virtual void LoadFromINI(
		SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual bool CanFireAt(CellStruct *pCoords);
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords);
};
#endif

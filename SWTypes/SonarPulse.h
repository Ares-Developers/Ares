#ifndef SUPERTYPE_EXT_SONAR_H
#define SUPERTYPE_EXT_SONAR_H

#include "..\SuperWeaponTypeExt.h"

#include <hash_map>

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
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual bool CanFireAt(CellStruct *pCoords);
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords);
};
#endif

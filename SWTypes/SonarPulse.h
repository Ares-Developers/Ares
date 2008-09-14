#ifndef SUPERTYPE_EXT_SONAR_H
#define SUPERTYPE_EXT_SONAR_H

#include <YRPP.h>
#include "..\Ares.h"
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

	virtual void SW_SonarPulse::LoadFromINI(
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual bool SW_SonarPulse::CanFireAt(CellStruct *pCoords);
	virtual bool SW_SonarPulse::Launch(SuperClass* pThis, CellStruct* pCoords);
};
#endif

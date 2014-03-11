#ifndef SUPERTYPE_EXT_SONAR_H
#define SUPERTYPE_EXT_SONAR_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_SonarPulse : public NewSWType
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
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
	virtual SuperWeaponFlags::Value Flags();

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

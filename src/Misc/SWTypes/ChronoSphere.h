#ifndef SUPERTYPE_EXT_CHRONOSPHERE_H
#define SUPERTYPE_EXT_CHRONOSPHERE_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_ChronoSphere : public NewSWType
{
	public:
		SW_ChronoSphere() : NewSWType()
			{ };

		virtual ~SW_ChronoSphere()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
};
#endif

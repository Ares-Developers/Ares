#ifndef SUPERTYPE_EXT_PROTECT_H
#define SUPERTYPE_EXT_PROTECT_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_Protect : public NewSWType
{
	public:
		SW_Protect() : NewSWType()
			{ };

		virtual ~SW_Protect()
			{ };

		virtual const char * GetTypeString()
			{ return "Protect"; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);

		virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
};
#endif

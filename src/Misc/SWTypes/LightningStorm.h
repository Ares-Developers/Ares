#ifndef SUPERTYPE_EXT_LSTORM_H
#define SUPERTYPE_EXT_LSTORM_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_LightningStorm : public NewSWType
{
	public:
		SW_LightningStorm() : NewSWType()
			{ };

		virtual ~SW_LightningStorm()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool AbortFire(SuperClass* pSW, bool IsPlayer);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
		virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
		virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;

		static SuperClass* CurrentLightningStorm;
};
#endif

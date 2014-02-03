#ifndef SUPERTYPE_EXT_REVEAL_H
#define SUPERTYPE_EXT_REVEAL_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_Reveal : public NewSWType
{
	public:
		SW_Reveal() : NewSWType()
			{ };

		virtual ~SW_Reveal()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);

		virtual int GetSound(const SWTypeExt::ExtData* pData) const override;
		virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

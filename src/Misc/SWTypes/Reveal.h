#ifndef SUPERTYPE_EXT_REVEAL_H
#define SUPERTYPE_EXT_REVEAL_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_Reveal : NewSWType
{
	public:
		SW_Reveal() : NewSWType()
			{ };

		virtual ~SW_Reveal()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
};
#endif

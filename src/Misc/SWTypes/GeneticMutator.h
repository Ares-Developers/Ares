#ifndef SUPERTYPE_EXT_MUTATOR_H
#define SUPERTYPE_EXT_MUTATOR_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_GeneticMutator : NewSWType
{
	public:
		SW_GeneticMutator() : NewSWType()
			{ };

		virtual ~SW_GeneticMutator()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
};
#endif

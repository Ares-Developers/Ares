#ifndef SUPERTYPE_EXT_MUTATOR_H
#define SUPERTYPE_EXT_MUTATOR_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_GeneticMutator : public NewSWType
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
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);

		virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
		virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
		virtual int GetSound(const SWTypeExt::ExtData* pData) const override;
};

#endif

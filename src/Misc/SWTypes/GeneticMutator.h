#ifndef SUPERTYPE_EXT_MUTATOR_H
#define SUPERTYPE_EXT_MUTATOR_H

#include "../SWTypes.h"

class SW_GeneticMutator : public NewSWType
{
public:
	SW_GeneticMutator() : NewSWType()
		{ };

	virtual ~SW_GeneticMutator() override
		{ };

	virtual const char * GetTypeString() override
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const override;
	virtual int GetSound(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;
};

#endif

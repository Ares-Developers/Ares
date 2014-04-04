#ifndef SUPERTYPE_EXT_NUKE_H
#define SUPERTYPE_EXT_NUKE_H

#include "../SWTypes.h"

class SW_NuclearMissile : public NewSWType
{
public:
	SW_NuclearMissile() : NewSWType()
		{ };

	virtual ~SW_NuclearMissile() override
		{ };

	virtual const char * GetTypeString() override
		{ return nullptr; }

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) override;
	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) override;
	virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer) override;
	virtual bool HandlesType(int type) override;
	virtual SuperWeaponFlags::Value Flags() override;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;

	static SuperWeaponTypeClass* CurrentNukeType;
};

#endif

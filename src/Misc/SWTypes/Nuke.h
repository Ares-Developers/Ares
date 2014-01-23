#ifndef SUPERTYPE_EXT_NUKE_H
#define SUPERTYPE_EXT_NUKE_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_NuclearMissile : NewSWType
{
	public:
		SW_NuclearMissile() : NewSWType()
			{ };

		virtual ~SW_NuclearMissile()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		static SuperWeaponTypeClass* CurrentNukeType;
};
#endif

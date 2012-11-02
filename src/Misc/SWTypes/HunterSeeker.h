#ifndef SUPERTYPE_EXT_HUNTERSEEKER_H
#define SUPERTYPE_EXT_HUNTERSEEKER_H

#include "../SWTypes.h"

class SW_HunterSeeker : public NewSWType
{
	public:
		SW_HunterSeeker() : NewSWType()
			{ };

		virtual ~SW_HunterSeeker()
			{ };

		virtual const char * GetTypeString()
			{ return "HunterSeeker"; }

		virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
};

#endif

#ifndef SUPERTYPE_EXT_DROPPOD_H
#define SUPERTYPE_EXT_DROPPOD_H

#include "../SWTypes.h"

class SW_DropPod : public NewSWType
{
	public:
		SW_DropPod() : NewSWType()
			{ };

		virtual ~SW_DropPod()
			{ };

		virtual const char * GetTypeString()
			{ return "DropPod"; }

		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
};
#endif

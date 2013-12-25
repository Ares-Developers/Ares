#ifndef BUILDINGTYPE_EXT_H
#define BUILDINGTYPE_EXT_H

#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <ColorScheme.h>
#include <HouseClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Constructs.h"

class AnimTypeExt
{
public:
	typedef AnimTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:

		enum {INVOKER, KILLER, VICTIM, NEUTRAL, RANDOM} MakeInfantryOwner;

		CustomPalette Palette;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			MakeInfantryOwner (INVOKER),
			Palette((BytePalette*)0x885780, DSurface::Primary, 53) // pointer to TEMPERAT_PAL (not the Convert!)
			{ };

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

	};

	static Container<AnimTypeExt> ExtMap;

	static void SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
//	static ExtData ExtMap;
};

#endif

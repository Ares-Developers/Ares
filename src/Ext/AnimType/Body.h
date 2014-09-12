#ifndef ANIMTYPE_EXT_H
#define ANIMTYPE_EXT_H

#include <AnimTypeClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"

class AnimClass;
class HouseClass;

class AnimTypeExt
{
public:
	typedef AnimTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		Valueable<OwnerHouseKind> MakeInfantryOwner;

		CustomPalette Palette;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			MakeInfantryOwner(OwnerHouseKind::Invoker),
			Palette(CustomPalette::PaletteMode::Temperate)
		{ };

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

	};

	static Container<AnimTypeExt> ExtMap;

	static OwnerHouseKind SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
};

#endif

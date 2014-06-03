#ifndef ANIMTYPE_EXT_H
#define ANIMTYPE_EXT_H

#include <AnimTypeClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Constructs.h"

class AnimClass;
class HouseClass;

class AnimTypeExt
{
public:
	typedef AnimTypeClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		enum class MakeInfantryHouse : int {
			Invoker = 0,
			Killer = 1,
			Victim = 2,
			Neutral = 3,
			Random = 4
		};

		MakeInfantryHouse MakeInfantryOwner;

		CustomPalette Palette;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			MakeInfantryOwner(MakeInfantryHouse::Invoker),
			Palette(CustomPalette::PaletteMode::Temperate)
		{ };

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

	};

	static Container<AnimTypeExt> ExtMap;

	static ExtData::MakeInfantryHouse SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
};

#endif

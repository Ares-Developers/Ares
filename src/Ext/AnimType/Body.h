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

	class ExtData : public Extension<AnimTypeClass>
	{
	public:
		Valueable<OwnerHouseKind> MakeInfantryOwner;

		CustomPalette Palette;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject),
			MakeInfantryOwner(OwnerHouseKind::Invoker),
			Palette(CustomPalette::PaletteMode::Temperate)
		{ };

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

	};

	static Container<AnimTypeExt> ExtMap;

	static OwnerHouseKind SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
};

#endif

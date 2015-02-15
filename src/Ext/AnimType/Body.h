#pragma once

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
	using base_type = AnimTypeClass;

	class ExtData : public Extension<AnimTypeClass>
	{
	public:
		Valueable<OwnerHouseKind> MakeInfantryOwner;

		CustomPalette Palette;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject),
			MakeInfantryOwner(OwnerHouseKind::Invoker),
			Palette(CustomPalette::PaletteMode::Temperate)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static Container<AnimTypeExt> ExtMap;

	static OwnerHouseKind SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
};

#ifndef ANIMTYPE_EXT_H
#define ANIMTYPE_EXT_H

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
			Palette()
			{ };

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr) {
		}

		virtual void SaveToStream(AresByteStream &pStm);

		virtual void LoadFromStream(AresByteStream &pStm, size_t &Offset);
	};

	static Container<AnimTypeExt> ExtMap;

	static void SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
//	static ExtData ExtMap;
};

#endif

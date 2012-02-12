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
			Palette()
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr) {
		}

		virtual void SaveToStream(AresSaveStream &pStm) {
			Extension<TT>::SaveToStream(pStm);
			AresSwizzle::SaveToStream(pStm, this->MakeInfantryOwner);
			AresSwizzle::SaveToStream(pStm, this->Palette);
		};

		virtual void LoadFromFile(IStream *pStm, size_t Size, size_t &Offset) {
			Extension<TT>::LoadFromFile(pStm, Size, Offset);
			AresSwizzle::LoadFromFile(pStm, this->MakeInfantryOwner, Size, Offset);
			AresSwizzle::LoadFromFile(pStm, this->Palette, Size, Offset);
		};

	};

	static Container<AnimTypeExt> ExtMap;

	static void SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller);
//	static ExtData ExtMap;
};

#endif

#ifndef BUILDINGTYPE_EXT_H
#define BUILDINGTYPE_EXT_H

#include <AnimTypeClass.h>

#include "../_Container.hpp"

class AnimTypeExt
{
public:
	typedef AnimTypeClass TT;

	enum eMakeInfOwner { makeInf_unchanged, makeInf_normal, makeInf_reverse };

	class ExtData : public Extension<TT>
	{
	public:

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr) {
		}

	};

	static Container<AnimTypeExt> ExtMap;
//	static ExtData ExtMap;
};

#endif

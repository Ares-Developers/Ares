#ifndef Infantry_EXT_H
#define Infantry_EXT_H

#include <CCINIClass.h>
#include <AbstractClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class AbstractExt
{
public:
	typedef AbstractClass TT;

	class ExtData : public Extension<TT>
	{
	public:

		DWORD LastChecksumTime;
		DWORD LastChecksum;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			LastChecksumTime(0),
			LastChecksum(0)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr) {
		}
	};

	static Container<AbstractExt> ExtMap;
};

#endif

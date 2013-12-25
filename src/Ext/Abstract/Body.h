#ifndef ABSTRACT_EXT_H
#define ABSTRACT_EXT_H

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

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			LastChecksumTime(0),
			LastChecksum(0)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}
	};

	static Container<AbstractExt> ExtMap;
};

#endif

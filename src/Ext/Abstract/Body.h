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

	class ExtData : public Extension<AbstractClass>
	{
	public:

		DWORD LastChecksumTime;
		DWORD LastChecksum;

		ExtData(AbstractClass* OwnerObject) : Extension<AbstractClass>(OwnerObject),
			LastChecksumTime(0),
			LastChecksum(0)
			{ };

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}
	};

	static Container<AbstractExt> ExtMap;
};

#endif

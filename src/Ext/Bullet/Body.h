#ifndef Bullet_EXT_H
#define Bullet_EXT_H

#include <CCINIClass.h>
#include <BulletClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

#define FOUNDATION_CUSTOM	0x7F

class BulletExt
{
public:
	typedef BulletClass TT;

	class ExtData : public Extension<TT>
	{
	public:


		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension(Canary, OwnerObject)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		bool DamageOccupants();

		virtual void InvalidatePointer(void *ptr) {
		}

	};

	static Container<BulletExt> ExtMap;
};

#endif

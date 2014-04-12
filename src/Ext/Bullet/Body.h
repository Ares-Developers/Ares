#ifndef Bullet_EXT_H
#define Bullet_EXT_H

#include <BulletClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

#define FOUNDATION_CUSTOM	0x7F

class SuperWeaponTypeClass;

class BulletExt
{
public:
	typedef BulletClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		SuperWeaponTypeClass *NukeSW;

		ExtData(TT* const OwnerObject) : Extension(OwnerObject),
			NukeSW (nullptr)
			{ };

		virtual ~ExtData() {
		}

		bool DamageOccupants();

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

	};

	static Container<BulletExt> ExtMap;
};

#endif

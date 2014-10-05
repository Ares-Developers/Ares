#ifndef Bullet_EXT_H
#define Bullet_EXT_H

#include <BulletClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class SuperWeaponTypeClass;

class BulletExt
{
public:
	typedef BulletClass TT;

	class ExtData : public Extension<BulletClass>
	{
	public:
		SuperWeaponTypeClass *NukeSW;

		ExtData(BulletClass* OwnerObject) : Extension(OwnerObject),
			NukeSW (nullptr)
			{ };

		virtual ~ExtData() = default;

		bool DamageOccupants();

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

	};

	static Container<BulletExt> ExtMap;
};

#endif

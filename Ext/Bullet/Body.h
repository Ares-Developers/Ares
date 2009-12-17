#ifndef Bullet_EXT_H
#define Bullet_EXT_H

#include <CCINIClass.h>
#include <BulletClass.h>

#include <Helpers/Macro.h>

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


		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) { };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual bool DamageOccupants();
	};

	static Container<BulletExt> ExtMap;
//	static ExtData ExtMap;
};

#endif

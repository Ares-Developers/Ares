#ifndef BULLETTYPE_EXT_H
#define BULLETTYPE_EXT_H

#include <CCINIClass.h>
#include <BulletTypeClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class BulletClass;
class ConvertClass;

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	class ExtData : public Extension<BulletTypeClass>
	{
	public:
		// solid
		Valueable<bool> SubjectToSolid;

		// firewall
		Valueable<bool> SubjectToFirewall;

		Valueable<bool> Parachuted;

		// added on 11.11.09 for #667 (part of Trenches)
		Valueable<bool> SubjectToTrenches; //! if false, this projectile/weapon *always* passes through to the occupants, regardless of UC.PassThrough

		// cache for the image animation's palette convert
		OptionalStruct<ConvertClass*> ImageConvert;

		Valueable<bool> Splits;
		Valueable<float> RetargetAccuracy;
		Valueable<float> AirburstSpread;
		Nullable<bool> AroundTarget; // aptly named, for both Splits and Airburst, defaulting to Splits
		Nullable<Leptons> BallisticScatterMin;
		Nullable<Leptons> BallisticScatterMax;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject),
			Splits(false),
			RetargetAccuracy(0.0f),
			AirburstSpread(1.5f),
			SubjectToSolid(false),
			SubjectToFirewall(true),
			Parachuted(false),
			SubjectToTrenches(true),
			ImageConvert()
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		ConvertClass* GetConvert();

		bool HasSplitBehavior();

		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const;
		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright) const;
	};

	static Container<BulletTypeExt> ExtMap;
};

#endif

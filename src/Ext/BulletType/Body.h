#pragma once

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

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		// solid
		Valueable<bool> SubjectToSolid;
		Valueable<int> Solid_Level;

		// firewall
		Valueable<bool> SubjectToFirewall;

		Valueable<bool> Parachuted;

		// added on 11.11.09 for #667 (part of Trenches)
		Valueable<bool> SubjectToTrenches; //! if false, this projectile/weapon *always* passes through to the occupants, regardless of UC.PassThrough

		// cache for the image animation's palette convert
		OptionalStruct<ConvertClass*> ImageConvert;

		Valueable<bool> Splits;
		Valueable<double> RetargetAccuracy;
		Valueable<double> AirburstSpread;
		Nullable<bool> AroundTarget; // aptly named, for both Splits and Airburst, defaulting to Splits
		Nullable<Leptons> BallisticScatterMin;
		Nullable<Leptons> BallisticScatterMax;

		Valueable<int> AnimLength;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject),
			Splits(false),
			RetargetAccuracy(0.0),
			AirburstSpread(1.5),
			SubjectToSolid(false),
			Solid_Level(0),
			SubjectToFirewall(true),
			Parachuted(false),
			SubjectToTrenches(true),
			ImageConvert()
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		ConvertClass* GetConvert();

		bool HasSplitBehavior();

		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const;
		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright) const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

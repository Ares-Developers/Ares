#include "Body.h"
#include "../TechnoType/Body.h"
#include "../AnimType/Body.h"
#include "../House/Body.h"
#include "../WeaponType/Body.h"
#include "../../Utilities/TemplateDef.h"

#include <BulletClass.h>

template<> const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
Container<BulletTypeExt> BulletTypeExt::ExtMap;

template<> BulletTypeExt::TT *Container<BulletTypeExt>::SavingObject = nullptr;
template<> IStream *Container<BulletTypeExt>::SavingStream = nullptr;

// =============================
// member funcs

void BulletTypeExt::ExtData::LoadFromINIFile(BulletTypeClass *pThis, CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	this->SubjectToSolid = pINI->ReadBool(pThis->ID, "SubjectToBuildings", this->SubjectToSolid);
	this->SubjectToFirewall = pINI->ReadBool(pThis->ID, "SubjectToFirewall", this->SubjectToFirewall);
	this->Parachuted = pINI->ReadBool(pThis->ID, "Parachuted", this->Parachuted);

	this->SubjectToTrenches = pINI->ReadBool(pThis->ID, "SubjectToTrenches", this->SubjectToTrenches);

	this->ImageConvert.clear();

	this->AV.Read(exINI, pThis->ID, "AV");

	this->AirburstSpread.Read(exINI, pThis->ID, "AirburstSpread");
	this->RetargetAccuracy.Read(exINI, pThis->ID, "RetargetAccuracy");
	this->Splits.Read(exINI, pThis->ID, "Splits");
	this->AroundTarget.Read(exINI, pThis->ID, "AroundTarget");

	this->BallisticScatterMin.Read(exINI, pThis->ID, "BallisticScatter.Min");
	this->BallisticScatterMax.Read(exINI, pThis->ID, "BallisticScatter.Max");
}

// get the custom palette of the animation this bullet type uses
ConvertClass* BulletTypeExt::ExtData::GetConvert()
{
	// cache the palette's convert
	if(this->ImageConvert.empty()) {
		ConvertClass* pConvert = nullptr;
		if(auto pAnimType = AnimTypeClass::Find(this->AttachedToObject->ImageFile)) {
			auto pData = AnimTypeExt::ExtMap.Find(pAnimType);
			pConvert = pData->Palette.GetConvert();
		}
		this->ImageConvert = pConvert;
	}

	return this->ImageConvert;
}

bool BulletTypeExt::ExtData::HasSplitBehavior()
{
	// behavior in FS: Splits defaults to Airburst.
	return this->AttachedToObject->Airburst || this->Splits;
}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const
{
	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	return this->CreateBullet(pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead,
		pWeapon->Speed, pExt->GetProjectileRange(), pWeapon->Bright);
}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner,
	int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright) const
{
	auto pBullet = this->AttachedToObject->CreateBullet(pTarget, pOwner, damage, pWarhead, speed, bright);

	if(pBullet) {
		pBullet->Range = range;
	}

	return pBullet;
}

int BulletTypeExt::ExtData::GetBallisticScatterMin(int default) const {
	if(this->BallisticScatterMin.isset()) {
		return Game::F2I(this->BallisticScatterMin * 256.0);
	}
	return default;
}

int BulletTypeExt::ExtData::GetBallisticScatterMax(int default) const {
	if(this->BallisticScatterMax.isset()) {
		return Game::F2I(this->BallisticScatterMax * 256.0);
	}
	return default;
}

// =============================
// container hooks

DEFINE_HOOK(46BDD9, BulletTypeClass_CTOR, 5)
{
	GET(BulletTypeClass*, pItem, EAX);

	BulletTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(46C890, BulletTypeClass_SDDTOR, 6)
{
	GET(BulletTypeClass*, pItem, ECX);

	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46C730, BulletTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46C6A0, BulletTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BulletTypeExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(46C722, BulletTypeClass_Load_Suffix, 4)
{
	BulletTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(46C74A, BulletTypeClass_Save_Suffix, 3)
{
	BulletTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(46C429, BulletTypeClass_LoadFromINI, A)
DEFINE_HOOK(46C41C, BulletTypeClass_LoadFromINI, A)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

#include "Body.h"
#include "../TechnoType/Body.h"
#include "../House/Body.h"

template<> const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
Container<BulletTypeExt> BulletTypeExt::ExtMap;

template<> BulletTypeExt::TT *Container<BulletTypeExt>::SavingObject = NULL;
template<> IStream *Container<BulletTypeExt>::SavingStream = NULL;

// =============================
// member funcs

void BulletTypeExt::ExtData::LoadFromINIFile(BulletTypeClass *pThis, CCINIClass* pINI)
{
	this->SubjectToSolid = pINI->ReadBool(pThis->ID, "SubjectToBuildings", this->SubjectToSolid);
	this->SubjectToFirewall = pINI->ReadBool(pThis->ID, "SubjectToFirewall", this->SubjectToFirewall);
	this->Parachuted = pINI->ReadBool(pThis->ID, "Parachuted", this->Parachuted);

	this->SubjectToTrenches = pINI->ReadBool(pThis->ID, "SubjectToTrenches", this->SubjectToTrenches);
}

// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);

	BulletTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x46C890, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ECX);

	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BulletTypeExt>::SavingObject = pItem;
	Container<BulletTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(0x46C722, BulletTypeClass_Load_Suffix, 0x4)
{
	BulletTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46C74A, BulletTypeClass_Save_Suffix, 0x3)
{
	BulletTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

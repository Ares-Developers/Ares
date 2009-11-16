#include "Body.h"
#include "../TechnoType/Body.h"
#include "../House/Body.h"

const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
Container<BulletTypeExt> BulletTypeExt::ExtMap;

BulletTypeExt::TT *Container<BulletTypeExt>::SavingObject = NULL;
IStream *Container<BulletTypeExt>::SavingStream = NULL;

// =============================
// member funcs

void BulletTypeExt::ExtData::LoadFromINIFile(BulletTypeClass *pThis, CCINIClass* pINI)
{
	this->SubjectToSolid = pINI->ReadBool(pThis->get_ID(), "SubjectToBuildings", this->SubjectToSolid);
	this->SubjectToFirewall = pINI->ReadBool(pThis->get_ID(), "SubjectToFirewall", this->SubjectToFirewall);
	this->SubjectToTrenches = pINI->ReadBool(pThis->get_ID(), "SubjectToTrenches", this->SubjectToTrenches);

	this->_Initialized = is_Completed;
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

DEFINE_HOOK(46C6A0, BulletTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(46C730, BulletTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(BulletTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BulletTypeExt>::SavingObject = pItem;
	Container<BulletTypeExt>::SavingStream = pStm;

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

DEFINE_HOOK(46C41C, BulletTypeClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(46C429, BulletTypeClass_LoadFromINI, A)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

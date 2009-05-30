#include "Body.h"
#include "..\TechnoType\Body.h"
#include "..\House\Body.h"

const DWORD Extension<BulletTypeClass>::Canary = 0xF00DF00D;
Container<BulletTypeExt> BulletTypeExt::ExtMap;

// =============================
// member funcs

void BulletTypeExt::ExtData::LoadFromINI(BulletTypeClass *pThis, CCINIClass* pINI)
{
	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	this->SubjectToSolid = pINI->ReadBool(pThis->get_ID(), "SubjectToBuildings", this->SubjectToSolid);
	this->SubjectToFirewall = pINI->ReadBool(pThis->get_ID(), "SubjectToFirewall", this->SubjectToFirewall);

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

DEFINE_HOOK(46C722, BulletTypeClass_Load, 4)
{
	GET_STACK(BulletTypeClass*, pItem, 0x8);
	GET_STACK(IStream*, pStm, 0xC);

	BulletTypeExt::ExtMap.Load(pItem, pStm);
	return 0;
}

DEFINE_HOOK(46C74A, BulletTypeClass_Save, 3)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExt::ExtMap.Save(pItem, pStm);
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

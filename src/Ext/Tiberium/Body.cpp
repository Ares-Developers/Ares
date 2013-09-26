#include "Body.h"

//Static init
template<> const DWORD Extension<TiberiumClass>::Canary = 0xB16B00B5;
Container<TiberiumExt> TiberiumExt::ExtMap;

template<> TiberiumExt::TT* Container<TiberiumExt>::SavingObject = nullptr;
template<> IStream* Container<TiberiumExt>::SavingStream = nullptr;

void TiberiumExt::ExtData::Initialize(TiberiumClass* pThis)
{
	//const char* pID = pThis->ID;
};

void TiberiumExt::ExtData::LoadFromINIFile(TiberiumClass* pThis, CCINIClass* pINI)
{
	const char* section = pThis->get_ID();

	INI_EX exINI(pINI);
}

// =============================
// load/save

void Container<TiberiumExt>::Save(TiberiumClass* pThis, IStream* pStm) {
	TiberiumExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		//ULONG out;
	}
}

void Container<TiberiumExt>::Load(TiberiumClass* pThis, IStream* pStm) {
	TiberiumExt::ExtData* pData = this->LoadKey(pThis, pStm);

	UNREFERENCED_PARAMETER(pData);
}

// =============================
// container hooks

DEFINE_HOOK(721876, TiberiumClass_CTOR, 5)
{
	GET(TiberiumClass*, pThis, ESI);

	TiberiumExt::ExtMap.FindOrAllocate(pThis);
	return 0;
}

DEFINE_HOOK(721880, TiberiumClass_DTOR, 5)
{
	GET(TiberiumClass*, pThis, ECX);

	TiberiumExt::ExtMap.Remove(pThis);
	return 0;
}

DEFINE_HOOK_AGAIN(7220D0, TiberiumClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(721E80, TiberiumClass_SaveLoad_Prefix, 7)
{
	GET_STACK(TiberiumExt::TT*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TiberiumExt>::SavingObject = pThis;
	Container<TiberiumExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(72208C, TiberiumClass_Load_Suffix, 7)
{
	TiberiumExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(72212C, TiberiumClass_Save_Suffix, 5)
{
	TiberiumExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(721CDC, TiberiumClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(721CE9, TiberiumClass_LoadFromINI, A)
DEFINE_HOOK(721C7B, TiberiumClass_LoadFromINI, A)
{
	GET(TiberiumClass*, pThis, ESI);
	GET(CCINIClass*, pINI, EBX);

	TiberiumExt::ExtMap.LoadFromINI(pThis, pINI);
	return 0;
}

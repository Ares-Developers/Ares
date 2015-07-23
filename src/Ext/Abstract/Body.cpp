#include "Body.h"

#include "../../Misc/SavegameDef.h"

template<> const DWORD Extension<AbstractClass>::Canary = 0xAB5005BA;
AbstractExt::ExtContainer AbstractExt::ExtMap;

// =============================
// load / save

template <typename T>
void AbstractExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->LastChecksumTime)
		.Process(this->LastChecksum);
}

void AbstractExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<AbstractClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AbstractExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<AbstractClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

AbstractExt::ExtContainer::ExtContainer() : Container("AbstractClass") {
}

AbstractExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(4101B6, AbstractClass_CTOR, 1)
{
	GET(AbstractClass*, pItem, EAX);

	AbstractExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4101F0, AbstractClass_DTOR, 6)
{
	GET(AbstractClass*, pItem, ECX);

	AbstractExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(410320, AbstractClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(410380, AbstractClass_SaveLoad_Prefix, 5)
{
	GET_STACK(AbstractClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AbstractExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(4103D6, AbstractClass_Load_Suffix, 4)
{
	AbstractExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(410372, AbstractClass_Save_Suffix, 5)
{
	AbstractExt::ExtMap.SaveStatic();
	return 0;
}
#endif

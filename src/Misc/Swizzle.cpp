#include "Swizzle.h"

AresSwizzle AresSwizzle::Instance;

HRESULT AresSwizzle::RegisterForChange(void **p) {
	if(p) {
		if(auto deref = *p) {
			this->Nodes.insert(std::make_pair(deref, p));
			*p = NULL;
		}
		return S_OK;
	}
	return E_POINTER;
}

HRESULT AresSwizzle::RegisterChange(void *was, void *is) {
	auto exist = this->Changes.find(was);
	if(exist == this->Changes.end()) {
		this->Changes[was] = is;
	} else if(exist->second != is) {
		Debug::DevLog(Debug::Error, "Pointer %p declared change to both %p AND %p!\n", was, exist->second, is);
	}
	return S_OK;
}

void AresSwizzle::ConvertNodes() const {
	void *lastFind(NULL);
	void *lastRes(NULL);
	for(auto it = this->Nodes.begin(); it != this->Nodes.end(); ++it) {
		if(lastFind != it->first) {
			auto change = this->Changes.find(it->first);
			if(change == this->Changes.end()) {
				Debug::DevLog(Debug::Error, "Pointer %p could not be remapped!\n", it->first);
			} else {
				lastFind = it->first;
				lastRes = change->second;
			}
		}
		if(auto p = it->second) {
			*p = lastRes;
		}
	}
}

void AresSwizzle::Clear() {
	this->Nodes.clear();
	this->Changes.clear();
}

DEFINE_HOOK(6CF350, SwizzleManagerClass_ConvertNodes, 0)
{
	AresSwizzle::Instance.ConvertNodes();
	AresSwizzle::Instance.Clear();

	return 0x6CF400;
}

DEFINE_HOOK(6CF2C0, SwizzleManagerClass_Here_I_Am, 0)
{
	GET_STACK(void *, oldP, 0x8);
	GET_STACK(void *, newP, 0xC);

	AresSwizzle::Instance.RegisterChange(oldP, newP);

	return 0x6CF316;
}

DEFINE_HOOK(6CF240, SwizzleManagerClass_Swizzle, 0)
{
	GET_STACK(void **, ptr, 0x8);

	AresSwizzle::Instance.RegisterForChange(ptr);

	return 0x6CF2B3;
}

DEFINE_HOOK(67D300, SaveGame_Start, 5)
{
	return 0;
}

DEFINE_HOOK(67E42E, SaveGame, 5)
{
	GET(HRESULT, Status, EAX);

	if(SUCCEEDED(Status)) {
		GET(IStream *, pStm, ESI);

		Status = Ares::SaveGameData(pStm);
		R->EAX<HRESULT>(Status);
	}

	return 0;
}

DEFINE_HOOK(67E730, LoadGame_Start, 5)
{
	Ares::LoadGame();
	return 0;
}

DEFINE_HOOK(67F7B9, LoadGame_End, 6)
{
	GET(IStream *, pStm, ESI);

	Ares::LoadGameData(pStm);

	return 0;
}

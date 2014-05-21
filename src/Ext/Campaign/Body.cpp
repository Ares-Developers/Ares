#include "Body.h"
#include "./../../Ares.h"
#include "./../../Ares.CRT.h"

//Static init
template<> const DWORD Extension<CampaignClass>::Canary = 0x22441133;
Container<CampaignExt> CampaignExt::ExtMap;
DynamicVectorClass<CampaignExt::ExtData*> CampaignExt::Array;
int CampaignExt::lastSelectedCampaign;

template<> CampaignExt::TT *Container<CampaignExt>::SavingObject = nullptr;
template<> IStream *Container<CampaignExt>::SavingStream = nullptr;

void CampaignExt::ExtData::Initialize(CampaignClass *pThis)
{
	if(!_strcmpi(pThis->ID, "ALL1")) {
		AresCRT::strCopy(this->HoverSound, "AlliedCampaignSelect");
	} else if (!_strcmpi(pThis->ID, "SOV1")) {
		AresCRT::strCopy(this->HoverSound, "SovietCampaignSelect");
	} else if (!_strcmpi(pThis->ID, "TUT1")) {
		AresCRT::strCopy(this->HoverSound, "BootCampSelect");
	}
};

void CampaignExt::ExtData::LoadFromINIFile(CampaignClass *pThis, CCINIClass *pINI)
{
	char* section = ((AbstractTypeClass*)pThis)->get_ID();

	this->DebugOnly = pINI->ReadBool(section, "DebugOnly", this->DebugOnly);

	if(pINI->ReadString(section, "HoverSound", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->HoverSound, Ares::readBuffer);
	}

	if(pINI->ReadString(section, "Summary", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->Summary, Ares::readBuffer);
	}
}

DEFINE_HOOK(46D090, CampaignClass_DTOR, 6)
{
	GET(CampaignClass*, pItem, ECX);
	CampaignExt::ExtMap.Remove(pItem);
	return 0;
}

// clear our own array before readding the items again
DEFINE_HOOK(46CE10, CampaignClass_LoadFromINIList, 5) {
	CampaignExt::Array.Clear();
	return 0;
}

// read Ares properties and maintain our own array
DEFINE_HOOK(46CD56, CampaignClass_LoadFromINI, 7)
{
	GET(CCINIClass*, pINI, EDI);
	GET(CampaignClass*, pThis, EBX);

	CampaignExt::ExtData* k = CampaignExt::ExtMap.FindOrAllocate(pThis);
	if(k) {
		CampaignExt::Array.AddItem(k);
		k->LoadFromINI(pThis, pINI);
	}
	return 0;
}

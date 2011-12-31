#include "Body.h"
#include "./../../Ares.h"
#include "./../../Ares.CRT.h"

//Static init
template<> const DWORD Extension<CampaignClass>::Canary = 0x22441133;
Container<CampaignExt> CampaignExt::ExtMap;
DynamicVectorClass<CampaignExt::ExtData*> CampaignExt::Array;
int CampaignExt::lastSelectedCampaign;

template<> CampaignExt::TT *Container<CampaignExt>::SavingObject = NULL;
template<> IStream *Container<CampaignExt>::SavingStream = NULL;

void CampaignExt::ExtData::Initialize(CampaignClass *pThis)
{
	if(!_strcmpi(pThis->ID, "ALL1")) {
		AresCRT::strCopy(this->HoverSound, "AlliedCampaignSelect", 0x1F);
	} else if (!_strcmpi(pThis->ID, "SOV1")) {
		AresCRT::strCopy(this->HoverSound, "SovietCampaignSelect", 0x1F);
	} else if (!_strcmpi(pThis->ID, "TUT1")) {
		AresCRT::strCopy(this->HoverSound, "BootCampSelect", 0x1F);
	}
};

void CampaignExt::ExtData::LoadFromINIFile(CampaignClass *pThis, CCINIClass *pINI)
{
	char* section = ((AbstractTypeClass*)pThis)->get_ID();

	this->DebugOnly = pINI->ReadBool(section, "DebugOnly", this->DebugOnly);

	if(pINI->ReadString(section, "HoverSound", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->HoverSound, Ares::readBuffer, 0x1F);
	}

	if(pINI->ReadString(section, "Summary", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->Summary, Ares::readBuffer, 0x20);
	}
}

DEFINE_HOOK(0x46D090, CampaignClass_DTOR, 0x6)
{
	GET(CampaignClass*, pItem, ECX);
	CampaignExt::ExtMap.Remove(pItem);
	return 0;
}

// clear our own array before readding the items again
DEFINE_HOOK(0x46CE10, CampaignClass_LoadFromINIList, 0x5) {
	CampaignExt::Array.Clear();
	return 0;
}

// read Ares properties and maintain our own array
DEFINE_HOOK(0x46CD56, CampaignClass_LoadFromINI, 0x7)
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
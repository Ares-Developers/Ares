#include "Body.h"
#include "./../../Ares.h"
#include "./../../Ares.CRT.h"

#include "./../../Utilities/TemplateDef.h"
#include "./../../Utilities/INIParser.h"

#include <algorithm>

//Static init
template<> const DWORD Extension<CampaignClass>::Canary = 0x22441133;
CampaignExt::ExtContainer CampaignExt::ExtMap;

DynamicVectorClass<CampaignExt::ExtData*> CampaignExt::Array;
int CampaignExt::lastSelectedCampaign;

void CampaignExt::ExtData::Initialize()
{
	auto pThis = this->OwnerObject();

	if(!_strcmpi(pThis->ID, "ALL1")) {
		this->HoverSound = "AlliedCampaignSelect";
	} else if(!_strcmpi(pThis->ID, "SOV1")) {
		this->HoverSound = "SovietCampaignSelect";
	} else if(!_strcmpi(pThis->ID, "TUT1")) {
		this->HoverSound = "BootCampSelect";
	}
};

void CampaignExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* section = this->OwnerObject()->get_ID();

	INI_EX exINI(pINI);

	this->DebugOnly.Read(exINI, section, "DebugOnly");

	this->HoverSound.Read(pINI, section, "HoverSound", "");

	this->Summary.Read(exINI, section, "Summary");
}

int CampaignExt::CountVisible() {
	return std::count_if(Array.begin(), Array.end(), [](CampaignExt::ExtData* pItem) {
		return pItem->IsVisible();
	});
}

// =============================
// container

CampaignExt::ExtContainer::ExtContainer() : Container("CampaignClass") {
}

CampaignExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

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

	if(auto k = CampaignExt::ExtMap.FindOrAllocate(pThis)) {
		CampaignExt::Array.AddItem(k);
		k->LoadFromINI(pINI);
	}
	return 0;
}

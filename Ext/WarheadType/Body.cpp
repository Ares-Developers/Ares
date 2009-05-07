#include "Body.h"
#include <WeaponTypeClass.h>
#include "..\..\Enum\ArmorTypes.h"

const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
Container<WarheadTypeExt> WarheadTypeExt::ExtMap;

hash_ionExt WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = NULL;

void WarheadTypeExt::ExtData::InitializeRuled(WarheadTypeClass *pThis)
{
	this->Temporal_WarpAway = RulesClass::Global()->WarpAway;

	this->_Initialized = is_Ruled;
};

void WarheadTypeExt::ExtData::LoadFromINI(WarheadTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	DEBUGLOG("Reading ext for %s\n", section);

//	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(pThis);
	if(!pINI->GetSection(section)) {
		return;
	}

	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	// writing custom verses parser just because
	char buffer[0x100];
	DEBUGLOG("\n[%s]Verses=", section);
	if(pINI->ReadString(section, "Verses", "", buffer, 0x100)) {
		DEBUGLOG("\t%s", buffer);
		int idx = 0;
		for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ",")) {
			DEBUGLOG("\n\t\tVerses #%d is %s", idx, cur);
			this->Verses[idx] = Conversions::Str2Armor(cur);
			DEBUGLOG("\n\t\tWhich converts to %lf", this->Verses[idx]);
			++idx;
			if(idx > 10) {
				break;
			}
		}
	}

	ArmorType::LoadForWarhead(pINI, pThis);

	if(pThis->MindControl) {
		this->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", this->MindControl_Permanent);
		this->Is_Custom |= this->MindControl_Permanent;
	}

	if(pThis->EMEffect) {
		this->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", this->EMP_Duration);
		this->Is_Custom |= 1;
	}

	this->IC_Duration = pINI->ReadInteger(section, "IronCurtain.Duration", this->IC_Duration);
	this->Is_Custom |= this->IC_Duration != 0;

	if(pThis->Temporal) {
		PARSE_BUF();

		PARSE_ANIM("Temporal.WarpAway", this->Temporal_WarpAway);
	}

	this->DeployedDamage = pINI->ReadDouble(section, "Damage.Deployed", this->DeployedDamage);

	this->Ripple_Radius = pINI->ReadInteger(section, "Ripple.Radius", this->Ripple_Radius);
};


// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E510, WarheadTypeClass_DTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ECX);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(75E2AE, WarheadTypeClass_Load, 7)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x14);
	GET_STACK(IStream*, pStm, 0x18);

	WarheadTypeExt::ExtMap.Load(pItem, pStm);
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save, 5)
{
	GET_STACK(WarheadTypeClass*, pItem, 0xC);
	GET_STACK(IStream*, pStm, 0x10);

	WarheadTypeExt::ExtMap.Save(pItem, pStm);
	return 0;
}

DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

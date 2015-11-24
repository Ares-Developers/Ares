#include "Body.h"
#include "../Building/Body.h"
#include "../Techno/Body.h"
#include "../../Utilities/TemplateDef.h"
#include <BulletClass.h>
#include <HouseClass.h>
#include <LaserDrawClass.h>
#include <WarheadTypeClass.h>

#include <vector>
#include <algorithm>

bool BuildingTypeExt::cPrismForwarding::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Stm
		.Process(this->Enabled)
		.Process(this->Targets, RegisterForChange)
		.Process(this->MaxFeeds)
		.Process(this->MaxChainLength)
		.Process(this->MaxNetworkSize)
		.Process(this->SupportModifier)
		.Process(this->DamageAdd)
		.Process(this->MyHeight)
		.Process(this->Intensity)
		.Process(this->ChargeDelay)
		.Process(this->ToAllies)
		.Process(this->BreakSupport)
		.Process(this->SupportWeaponIndex)
		.Process(this->EliteSupportWeaponIndex)
		.Success();
}

bool BuildingTypeExt::cPrismForwarding::Save(AresStreamWriter &Stm) const {
	return Stm
		.Process(this->Enabled)
		.Process(this->Targets)
		.Process(this->MaxFeeds)
		.Process(this->MaxChainLength)
		.Process(this->MaxNetworkSize)
		.Process(this->SupportModifier)
		.Process(this->DamageAdd)
		.Process(this->MyHeight)
		.Process(this->Intensity)
		.Process(this->ChargeDelay)
		.Process(this->ToAllies)
		.Process(this->BreakSupport)
		.Process(this->SupportWeaponIndex)
		.Process(this->EliteSupportWeaponIndex)
		.Success();
}

void BuildingTypeExt::cPrismForwarding::Initialize(BuildingTypeClass *pThis) {
	this->Enabled = EnabledState::No;
	if (pThis == RulesClass::Instance->PrismType) {
		this->Enabled = EnabledState::Yes;
	}
	this->Targets.push_back(pThis);
}

void BuildingTypeExt::cPrismForwarding::LoadFromINIFile(BuildingTypeClass *pThis, CCINIClass* pINI) {
	const char * pID = pThis->ID;
	if(pINI->ReadString(pID, "PrismForwarding", "", Ares::readBuffer)) {
		if((_strcmpi(Ares::readBuffer, "yes") == 0) || (_strcmpi(Ares::readBuffer, "true") == 0)) {
			this->Enabled = EnabledState::Yes;
		} else if(_strcmpi(Ares::readBuffer, "forward") == 0) {
			this->Enabled = EnabledState::Forward;
		} else if(_strcmpi(Ares::readBuffer, "attack") == 0) {
			this->Enabled = EnabledState::Attack;
		} else if((_strcmpi(Ares::readBuffer, "no") == 0) || (_strcmpi(Ares::readBuffer, "false"))== 0) {
			this->Enabled = EnabledState::No;
		}
	}

	if (this->Enabled != EnabledState::No) {
		INI_EX exINI(pINI);

		this->Targets.Read(exINI, pID, "PrismForwarding.Targets");
		this->MaxFeeds.Read(exINI, pID, "PrismForwarding.MaxFeeds");
		this->MaxChainLength.Read(exINI, pID, "PrismForwarding.MaxChainLength");
		this->MaxNetworkSize.Read(exINI, pID, "PrismForwarding.MaxNetworkSize");
		this->SupportModifier.Read(exINI, pID, "PrismForwarding.SupportModifier");
		this->DamageAdd.Read(exINI, pID, "PrismForwarding.DamageAdd");
		this->ToAllies.Read(exINI, pID, "PrismForwarding.ToAllies");
		this->MyHeight.Read(exINI, pID, "PrismForwarding.MyHeight");
		this->BreakSupport.Read(exINI, pID, "PrismForwarding.BreakSupport");
		this->Intensity.Read(exINI, pID, "PrismForwarding.Intensity");
		this->ChargeDelay.Read(exINI, pID, "PrismForwarding.ChargeDelay");
		
		if(this->ChargeDelay < 1) {
			Debug::Log("[Developer Error] %s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, this->ChargeDelay.Get());
			this->ChargeDelay = 1;
		}

		auto SuperWH = RulesClass::Instance->C4Warhead;
		if(!SuperWH) {
			SuperWH = WarheadTypeClass::Find("Super");
		}

		auto const ReadSupportWeapon = [=, &exINI]
			(int* pSetting, const char* const pKey, bool const elite)
		{
			if(exINI.ReadString(pID, pKey)) {
				if(auto const pWeapon = WeaponTypeClass::FindOrAllocate(exINI.value())) {
					auto const idxWeapon = *pSetting != -1
						? *pSetting : this->GetUnusedWeaponSlot(pThis, elite);
					if(idxWeapon == -1) {
						Debug::FatalErrorAndExit(
							"BuildingType [%s] is a Prism Tower however there "
							"are no free\nweapon slots to assign the %ssupport "
							"weapon to.", pThis->ID, elite ? "elite " : "");
					}

					*pSetting = idxWeapon;

					if(!pWeapon->Warhead) {
						pWeapon->Warhead = SuperWH;
					}
					pWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this

					auto& Weapon = pThis->GetWeapon(idxWeapon, elite);
					Weapon.WeaponType = pWeapon;

					//now get the FLH
					auto supportFLH = &pThis->Weapon[13 + elite].FLH; //AlternateFLH0 or 1
					if(*supportFLH == CoordStruct::Empty) {
						//assuming that, for Prism Towers, this means the FLH was not set.
						supportFLH = &pThis->GetWeapon(0, elite).FLH; //[Elite]Primary
					}
					Weapon.FLH = *supportFLH;
				}
			}
		};

		ReadSupportWeapon(&this->SupportWeaponIndex, "PrismForwarding.SupportWeapon", false);
		ReadSupportWeapon(&this->EliteSupportWeaponIndex, "PrismForwarding.EliteSupportWeapon", true);
	}
}

signed int BuildingTypeExt::cPrismForwarding::GetUnusedWeaponSlot(BuildingTypeClass *pThis, bool elite) {
	for(auto idxWeapon = 2u; idxWeapon < 13u; ++idxWeapon) { //13-18 is AlternateFLH0-4
		auto Weapon = pThis->GetWeapon(idxWeapon, elite).WeaponType;
		if(!Weapon) {
			return static_cast<int>(idxWeapon);
		}
	}
	return -1;
}

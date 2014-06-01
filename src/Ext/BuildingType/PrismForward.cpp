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

void BuildingTypeExt::cPrismForwarding::Initialize(BuildingTypeClass *pThis) {
	this->Enabled = NO;
	if (pThis == RulesClass::Instance->PrismType) {
		this->Enabled = YES;
	}
	this->Targets.push_back(pThis);
}

void BuildingTypeExt::cPrismForwarding::LoadFromINIFile(BuildingTypeClass *pThis, CCINIClass* pINI) {
	const char * pID = pThis->ID;
	if(pINI->ReadString(pID, "PrismForwarding", "", Ares::readBuffer, Ares::readLength)) {
		if((_strcmpi(Ares::readBuffer, "yes") == 0) || (_strcmpi(Ares::readBuffer, "true") == 0)) {
			this->Enabled = YES;
		} else if(_strcmpi(Ares::readBuffer, "forward") == 0) {
			this->Enabled = FORWARD;
		} else if(_strcmpi(Ares::readBuffer, "attack") == 0) {
			this->Enabled = ATTACK;
		} else if((_strcmpi(Ares::readBuffer, "no") == 0) || (_strcmpi(Ares::readBuffer, "false"))== 0) {
			this->Enabled = NO;
		}
	}

	if (this->Enabled != NO) {
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
		
		int ChargeDelay = pINI->ReadInteger(pID, "PrismForwarding.ChargeDelay", this->ChargeDelay);
		if (ChargeDelay >= 1) {
			this->ChargeDelay.Set(ChargeDelay);
		} else {
			Debug::Log("[Developer Error] %s has an invalid PrismForwarding.ChargeDelay (%d), overriding to 1.\n", pThis->ID, ChargeDelay);
		}

		auto SuperWH = RulesClass::Instance->C4Warhead;
		if(!SuperWH) {
			SuperWH = WarheadTypeClass::Find("Super");
		}

		if(pINI->ReadString(pID, "PrismForwarding.SupportWeapon", "", Ares::readBuffer, Ares::readLength)) {
			if (WeaponTypeClass *cWeapon = WeaponTypeClass::FindOrAllocate(Ares::readBuffer)) {
				int idxWeapon = this->GetUnusedWeaponSlot(pThis, 0); //rookie weapons
				if (idxWeapon == -1) {
					Debug::FatalErrorAndExit(
						"BuildingType [%s] is a Prism Tower however there are no free\n"
						"weapon slots to assign the support weapon to.", pThis->ID);
				}
				this->SupportWeaponIndex = idxWeapon;
				if(!cWeapon->Warhead) {
					cWeapon->Warhead = SuperWH;
				}
				cWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this
				CoordStruct supportFLH;
				pThis->set_Weapon(idxWeapon, cWeapon);
				//now get the FLH
				supportFLH = pThis->get_WeaponFLH(13); //AlternateFLH0
				if (supportFLH.X == 0 && supportFLH.Y == 0 && supportFLH.Z == 0) {
					//assuming that, for Prism Towers, this means the FLH was not set.
					supportFLH = pThis->get_WeaponFLH(0); //Primary
				}
				pThis->set_WeaponFLH(idxWeapon, supportFLH);
			}
		}

		if(pINI->ReadString(pID, "PrismForwarding.EliteSupportWeapon", "", Ares::readBuffer, Ares::readLength)) {
			if (WeaponTypeClass *cWeapon = WeaponTypeClass::FindOrAllocate(Ares::readBuffer)) {
				int idxWeapon = this->GetUnusedWeaponSlot(pThis, 1); //elite weapons
				if (idxWeapon == -1) {
					Debug::FatalErrorAndExit(
						"BuildingType [%s] is a Prism Tower however there are no free\n"
						"weapon slots to assign the elite support weapon to.", pThis->ID);
				}
				this->EliteSupportWeaponIndex = idxWeapon;
				if(!cWeapon->Warhead) {
					cWeapon->Warhead = SuperWH;
				}
				cWeapon->NeverUse = true; //the modder shouldn't be expected to have to set this
				CoordStruct supportFLH;
				pThis->set_EliteWeapon(idxWeapon, cWeapon);
				//now get the FLH
				supportFLH = pThis->get_WeaponFLH(14); //AlternateFLH1
				if (supportFLH.X == 0 && supportFLH.Y == 0 && supportFLH.Z == 0) {
					//assuming that, for Prism Towers, this means the FLH was not set.
					supportFLH = pThis->get_EliteWeaponFLH(0); //ElitePrimary
				}
				pThis->set_EliteWeaponFLH(idxWeapon, supportFLH);
			}
		}

	}
}

signed int BuildingTypeExt::cPrismForwarding::GetUnusedWeaponSlot(BuildingTypeClass *pThis, bool elite) {
	for(int idxWeapon = 2; idxWeapon < 13; ++idxWeapon) { //13-18 is AlternateFLH0-4
		auto Weapon = elite ? pThis->get_EliteWeapon(idxWeapon) : pThis->get_Weapon(idxWeapon);
		if(!Weapon) {
			return idxWeapon;
		}
	}
	return -1;
}

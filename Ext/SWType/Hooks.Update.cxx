#include "Body.h"
#include <MouseClass.h>

#include <algorithm>
#include <functional>

DEFINE_HOOK(50AF10, HouseClass_CheckSWs, 5)
{
	GET(HouseClass *, pThis, ECX);

	bool isPlayer = pThis == HouseClass::Player;
	for(int idxSW = 0; idxSW < pThis->Supers.Count; ++idxSW) {
		SuperClass *pSW = pThis->Supers[idxSW];
		SuperWeaponTypeClass * pSWType = pSW->Type;
		if(pSW->Granted) {
			bool skip = !pSW->unknown_bool_60 || pSW->Quantity && pSW->Granted;
			if(!skip || pThis->Defeated) { // don't ask me...
				bool PowerSourced = false;
				bool Available = false;
				if(!pThis->Defeated) {

					for(int idxBld = 0; idxBld < BuildingClass::Array->Count; ++idxBld) {
						BuildingClass * pBld = BuildingClass::Array->GetItem(idxBld);
						if(pBld->IsAlive && !pBld->InLimbo) {
							if(pBld->Owner == pThis) {

								for(int i = 0; i < 3; ++i) {
									if(BuildingTypeClass *Upgrade = pBld->Upgrades[i]) {
										if(Upgrade->SuperWeapon == idxSW || Upgrade->SuperWeapon2 == idxSW) {
											Available = true;
											if(!PowerSourced) {
												PowerSourced = pBld->HasPower;
											}
										}
									}
								}

								if(pBld->FirstActiveSWIdx() == idxSW || pBld->SecondActiveSWIdx() == idxSW) {
									Available = true;
									if(!PowerSourced) {
										PowerSourced = pBld->HasPower;
									}
								}

								if(Available && PowerSourced) {
									break;
								}
							}
						}
					}


				}

				if(!Unsorted::SWAllowed) {
					if(pSWType->DisableableFromShell) {
						Available = false;
					}
				}

				int HavePower = pThis->PowerOutput;
				int NeedPower = pThis->PowerDrain;
				if(HavePower < NeedPower) {
					if(NeedPower) {
						if(!HavePower || (HavePower * 1.0 / NeedPower < 1.0)) { // and this is how you get BONUS POINTS
							PowerSourced = false;
						}
					}
				}

				int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, pSWType->GetArrayIndex(), 0);

				// bua hahahaha
				std::tr1::function<void ()> UpdateUI = [=]() {
					if(isPlayer) {
						if(Unsorted::CurrentSWType == idxSW) {
							Unsorted::CurrentSWType = -1;
						}
						MouseClass::Instance->RepaintSidebar(idxTab);
					}
					pThis->ShouldRecheckTechTree = true;
				};

				if(!Available || pThis->Defeated) {
					if(pSW->Lose() && HouseClass::Player) {
						UpdateUI();
					}
					continue;
				}

				if(!PowerSourced) {
					if(pSW->IsPowered() && pSW->SetOnHold(1)) {
						UpdateUI();
						continue;
					}
				}
				if(PowerSourced && pSW->SetOnHold(0)) {
					UpdateUI();
				}
			}
		}

	}

	return 0x50B1CA;
}

#include <PCX.h>

#include "Body.h"
#include "../../Misc/SWTypes.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"
#include "../../Ares.h"
#include "../../Ares.CRT.h"

template<> const DWORD Extension<SuperWeaponTypeClass>::Canary = 0x55555555;
Container<SWTypeExt> SWTypeExt::ExtMap;

template<> SWTypeExt::TT *Container<SWTypeExt>::SavingObject = NULL;
template<> IStream *Container<SWTypeExt>::SavingStream = NULL;

SuperWeaponTypeClass *SWTypeExt::CurrentSWType = NULL;

void SWTypeExt::ExtData::InitializeConstants(SuperWeaponTypeClass *pThis)
{
	if(!NewSWType::Array.Count) {
		NewSWType::Init();
	}

	MouseCursor *Cursor = &this->SW_Cursor;
	Cursor->Frame = 53; // Attack
	Cursor->Count = 5;
	Cursor->Interval = 5; // test?
	Cursor->MiniFrame = 52;
	Cursor->MiniCount = 1;
	Cursor->HotX = hotspx_center;
	Cursor->HotY = hotspy_middle;

	Cursor = &this->SW_NoCursor;
	Cursor->Frame = 0;
	Cursor->Count = 1;
	Cursor->Interval = 5;
	Cursor->MiniFrame = 1;
	Cursor->MiniCount = 1;
	Cursor->HotX = hotspx_center;
	Cursor->HotY = hotspy_middle;
}

void SWTypeExt::ExtData::InitializeRuled(SuperWeaponTypeClass *pThis)
{
	this->SpyPlane_TypeIndex = AircraftTypeClass::FindIndex("SPYP");
	this->Nuke_Siren = RulesClass::Global()->DigSound;

	this->AmerParaDrop.Clear();
	this->AmerParaDropNum.Clear();
	if(pThis->Type == 6) {
		for(int i = 0; i < RulesClass::Instance->AmerParaDropInf.Count; ++i) {
			this->AmerParaDrop.AddItem((RulesClass::Instance->AmerParaDropInf.GetItem(i)));
		}

		for(int i = 0; i < RulesClass::Instance->AmerParaDropNum.Count; ++i) {
			this->AmerParaDropNum.AddItem(RulesClass::Instance->AmerParaDropNum.GetItem(i));
		}
	}

	// set up paradrop members
	int type = this->AttachedToObject->Type;
	if((type == 5) || (type == 6)) {
		// create an array to hold a vector for each side and country
		int max = SideClass::Array->Count + HouseTypeClass::Array->Count + 1;
		ParaDrop = new DynamicVectorClass<ParadropPlane*>[max];
	}
}

void SWTypeExt::ExtData::LoadFromINIFile(SuperWeaponTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	this->SpyPlane_Count.Read(&exINI, section, "SpyPlane.Count");

	this->SpyPlane_TypeIndex.Read(&exINI, section, "SpyPlane.Type");

	this->SpyPlane_Mission.Read(&exINI, section, "SpyPlane.Mission");

	this->Nuke_Siren.Read(&exINI, section, "Nuke.Sound");
	this->EVA_Ready.Read(&exINI, section, "EVA.Ready");
	this->EVA_Activated.Read(&exINI, section, "EVA.Activated");
	this->EVA_Detected.Read(&exINI, section, "EVA.Detected");

	if(exINI.ReadString(section, "Action") && !strcmp(exINI.value(), "Custom")) {
		pThis->Action = SW_YES_CURSOR;
	}

	if(exINI.ReadString(section, "Type")) {
		int customType = NewSWType::FindIndex(exINI.value());
		if(customType > -1) {
			pThis->Type = customType;
		}
	}

	this->SW_FireToShroud.Read(&exINI, section, "Super.FireIntoShroud");
	this->SW_AutoFire.Read(&exINI, section, "Super.AutoFire");
	this->SW_RadarEvent.Read(&exINI, section, "Super.CreateRadarEvent");

	this->Money_Amount.Read(&exINI, section, "Money.Amount");

	this->SW_Anim.Parse(&exINI, section, "SW.Animation");
	this->SW_AnimHeight.Read(&exINI, section, "SW.AnimationHeight");

	this->SW_Sound.Read(&exINI, section, "SW.Sound");

	this->SW_Cursor.Read(&exINI, section, "Cursor");
	this->SW_NoCursor.Read(&exINI, section, "NoCursor");

	int Type = pThis->Type - FIRST_SW_TYPE;
	if(Type >= 0 && Type < NewSWType::Array.Count ) {
		NewSWType *swt = NewSWType::GetNthItem(pThis->Type);
		swt->LoadFromINI(this, pThis, pINI);
	}

	this->CameoPal.LoadFromINI(pINI, pThis->ID, "SidebarPalette");

	this->ParaDropPlane.Read(&exINI, section, "ParaDrop.Aircraft");

	char* p = NULL;
	if(pINI->ReadString(pThis->ID, "ParaDrop.Types", "", Ares::readBuffer, Ares::readLength)) {
		this->AmerParaDrop.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			TechnoTypeClass* pTT = UnitTypeClass::Find(p);

			if(!pTT) {
				pTT = InfantryTypeClass::Find(p);
			}

			if(pTT) {
				this->AmerParaDrop.AddItem(pTT);
			}
		}
	}

	if(pINI->ReadString(pThis->ID, "ParaDrop.Num", "", Ares::readBuffer, Ares::readLength)) {
		this->AmerParaDropNum.Clear();

		for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
			this->AmerParaDropNum.AddItem(atoi(p));
		}
	}

	if(pINI->ReadString(section, "SidebarPCX", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->SidebarPCX, Ares::readBuffer, 0x20);
		PCX::Instance->LoadFile(this->SidebarPCX);
	}

	char base[0x40];

	auto CreateParaDropBase = [](char* pID, char* pBuffer) {
		// but a string like "Paradrop.Americans" into the buffer
		if(pBuffer) {
			AresCRT::strCopy(pBuffer, "ParaDrop", 9);
			if(pID && strlen(pID)) {
				AresCRT::strCopy(&pBuffer[8], ".", 2);
				AresCRT::strCopy(&pBuffer[9], pID, 0x18);
			}
		}
	};

	auto ParseParaDrop = [&](char* pID, int Plane, ParadropPlane* pPlane) {
		if(pPlane) {
			// create the plane part of this request. this will be
			// an empty string for the first plane for this is the default.
			char plane[0x10] = "";
			if(Plane) {
				AresCRT::strCopy(plane, ".Plane", 0x10);
				_itoa(Plane + 1, &plane[6], 10);
			}
		
			// construct the full tag name base
			char base[0x40], key[0x40];
			_snprintf(base, 0x40, "%s%s", pID, plane);

			// parse the plane contents
			_snprintf(key, 0x40, "%s.Aircraft", base);
			if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
				pPlane->pAircraft = AircraftTypeClass::Find(Ares::readBuffer);
			}

			// a list of UnitTypes and InfantryTypes
			_snprintf(key, 0x40, "%s.Types", base);
			if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
				pPlane->pTypes.Clear();

				for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
					TechnoTypeClass* pTT = UnitTypeClass::Find(p);

					if(!pTT) {
						pTT = InfantryTypeClass::Find(p);
					}

					if(pTT) {
						pPlane->pTypes.AddItem(pTT);
					}
				}
			}

			// the number how many times each item is created
			_snprintf(key, 0x40, "%s.Num", base);
			if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
				pPlane->pNum.Clear();

				for(p = strtok(Ares::readBuffer, Ares::readDelims); p && *p; p = strtok(NULL, Ares::readDelims)) {
					pPlane->pNum.AddItem(atoi(p));
				}
			}
		}
	};

	auto GetSpecificParadrop = [&](char *pID, int defCount, DynamicVectorClass<ParadropPlane*>* ret) {
		// get the number of planes for this house or side
		char key[0x40];
		_snprintf(key, 0x40, "%s.Count", pID);
		int count = pINI->ReadInteger(section, key, defCount);

		// parse every plane
		ret->SetCapacity(count, NULL);
		for(int i=0; i<count; ++i) {
			if(i>=ret->Count) {
				ret->AddItem(new ParadropPlane());
			}
			ParseParaDrop(base, i, ret->Items[i]);
		}
	};

	int type = this->AttachedToObject->Type;
	if((type == 5) || (type == 6)) {

		// default
		CreateParaDropBase(NULL, base);
		GetSpecificParadrop(base, 1, &ParaDrop[0]);

		// put all sides into the array
		for(int i=0; i<SideClass::Array->Count; ++i) {
			SideClass *pSide = SideClass::Array->GetItem(i);
			CreateParaDropBase(pSide->ID, base);
			GetSpecificParadrop(base, ParaDrop[0].Count, &ParaDrop[i+1]);
		}

		// put all countries into the array
		for(int i=0; i<HouseTypeClass::Array->Count; ++i) {
			HouseTypeClass *pTHouse = HouseTypeClass::Array->GetItem(i);
			CreateParaDropBase(pTHouse->ID, base);
			GetSpecificParadrop(base, ParaDrop[pTHouse->SideIndex+1].Count, &ParaDrop[i+SideClass::Array->Count+1]);
		}
	}
}

AircraftTypeClass* SWTypeExt::ExtData::GetParadropPlane(HouseClass* pHouse, int plane=0) {
	// tries to get the house's default plane and falls back to
	// the side's default plane. then the general default plane.
	if(pHouse) {
		int indices[3] = {0, 0, 0};
		indices[0] = pHouse->Type->ArrayIndex + SideClass::Array->Count;
		indices[1] = pHouse->Type->SideIndex + 1;

		// try the specific plane and then, if that fails, the first one.
		for(int j=1; j>=0; --j) {
			for(int i=0; i<3; ++i) {
				DynamicVectorClass<ParadropPlane*> planes = this->ParaDrop[indices[i]];
				if(planes.Count) {
					int index = plane * j;
					if(planes.ValidIndex(index)) {
						if(ParadropPlane* pPlane = planes.GetItem(index)) {
							if(AircraftTypeClass* pTAircraft = pPlane->pAircraft) {
								return pTAircraft;
							}
						}
					}
				}
			}
		}

		// get the house's default paradrop plane
		if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
			return pData->GetParadropPlane();
		}

		// this should not happen
		Debug::Log("[GetParadropPlane] Superweapon %s could not determine an aircraft for house %s.\n", this->AttachedToObject->ID, pHouse->Type->ID);
	}

	return NULL;
}

int SWTypeExt::ExtData::GetParadropCount(HouseClass* pHouse) {
	int index = 0;
	
	if(pHouse) {
		index = pHouse->Type->ArrayIndex + SideClass::Array->Count;
	}

	return ParaDrop[index].Count;
}

bool SWTypeExt::ExtData::GetParadropContents(HouseClass* pHouse, int iPlane, TypeList<TechnoTypeClass*>** ppTypes, TypeList<int>** ppNum) {
	if(ppTypes && ppNum) {
		if(pHouse) {
			int indices[3] = {0, 0, 0};
			indices[0] = pHouse->Type->ArrayIndex + SideClass::Array->Count;
			indices[1] = pHouse->Type->SideIndex + 1;

			// try the specific plane and then, if that fails, the first one.
			for(int j=1; j>=0; --j) {
				for(int i=0; i<3; ++i) {
					DynamicVectorClass<ParadropPlane*> planes = this->ParaDrop[indices[i]];
					if(planes.Count) {
						int index = iPlane * j;
						if(planes.ValidIndex(index)) {
							if(ParadropPlane* pPlane = planes.GetItem(index)) {
								if(AircraftTypeClass* pTAircraft = pPlane->pAircraft) {
									//return pTAircraft;
								}
							}
						}
					}
				}
			}

			// get the house's default paradrop plane
			if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
				return pData->GetParadropContent(ppTypes, ppNum);
			}

			// this should not happen
			Debug::Log("[GetParadropContents] Superweapon %s could not determine contents for house %s, plane %d.\n", this->AttachedToObject->ID, pHouse->Type->ID, iPlane);
		}

		*ppTypes = NULL;
		*ppNum = NULL;
	}

	return false;
}

bool SWTypeExt::ExtData::SendParadrop(HouseClass* pHouse, CellClass* pCell) {
	// sanity
	if(!pHouse || !pCell) {
		return false;
	}

	// these are fallback values if the SW doesn't define them
	AircraftTypeClass* pFallbackPlane = NULL;
	TypeList<TechnoTypeClass*> *pFallbackTypes = NULL;
	TypeList<int> *pFallbackNum = NULL;

	// how many planes shall we launch?
	int count = this->GetParadropCount(pHouse);

	// assemble each plane and its contents
	for(int i=0; i<count; ++i) { // i = index of plane
		TypeList<TechnoTypeClass*> *pParaDrop = NULL;
		TypeList<int> *pParaDropNum = NULL;
		AircraftTypeClass* pParaDropPlane = NULL;

		int indices[3] = {0, 0, 0};
		indices[0] = pHouse->Type->ArrayIndex + SideClass::Array->Count + 1;
		indices[1] = pHouse->Type->SideIndex + 1;

		// try the planes in order of precedence:
		// * country, explicit plane
		// * side, explicit plane
		// * default, explict plane
		// * country, default plane
		// * side, default plane
		// * default, default plane
		// * fill spaces with data from house/side/rules
		for(int j=1; j>=0; --j) { // factor 1 or 0: "plane * j" => "plane" or "0" (default)
			for(int k=0; k<3; ++k) { // index in the indices array

				// only do something if there is data missing
				if(!(pParaDrop && pParaDropNum && pParaDropPlane)) {
					// get the country/side-specific plane list
					DynamicVectorClass<ParadropPlane*> *planes = &this->ParaDrop[indices[k]];

					// get the plane at specified index
					int index = i * j;
					if(planes->ValidIndex(index)) {
						if(ParadropPlane* pPlane = planes->GetItem(index)) {

							// get the contents, if not already set
							if(!pParaDrop || !pParaDropNum) {
								if((pPlane->pTypes.Count != 0) && (pPlane->pNum.Count != 0)) {
									pParaDrop = &pPlane->pTypes;
									pParaDropNum = &pPlane->pNum;
								}
							}

							// get the airplane, if it isn't set already
							if(!pParaDropPlane) {
								if(AircraftTypeClass* pTAircraft = pPlane->pAircraft) {
									pParaDropPlane = pTAircraft;
								}
							}
						}
					}
				}
			}
		}

		// fallback for types and nums
		if(!pParaDrop || !pParaDropNum) {
			if(!pFallbackTypes || !pFallbackNum) {
				if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
					pData->GetParadropContent(&pFallbackTypes, &pFallbackNum);
				}
			}

			pParaDrop = pFallbackTypes;
			pParaDropNum = pFallbackNum;
		}


		// house fallback for the plane
		if(!pParaDropPlane) {
			if(!pFallbackPlane) {
				if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pHouse->Type)) {
					pFallbackPlane = pData->GetParadropPlane();
				}
			}

			pParaDropPlane = pFallbackPlane;
		}

		// finally, send the plane
		if(pParaDrop && pParaDropNum && pParaDropPlane) {
			Ares::SendPDPlane(
				pHouse,
				pCell,
				pParaDropPlane,
				pParaDrop,
				pParaDropNum);
		}
	}

	return true;
}

bool __stdcall SWTypeExt::SuperClass_Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pThis->Type);

	if(pData->EVA_Activated != -1) {
		VoxClass::PlayIndex(pData->EVA_Activated);
	}

	int Money_Amount = pData->Money_Amount;
	if(Money_Amount > 0) {
		DEBUGLOG("House %d gets %d credits\n", pThis->Owner->ArrayIndex, Money_Amount);
		pThis->Owner->GiveMoney(Money_Amount);
	} else if(Money_Amount < 0) {
		DEBUGLOG("House %d loses %d credits\n", pThis->Owner->ArrayIndex, -Money_Amount);
		pThis->Owner->TakeMoney(-Money_Amount);
	}

	CoordStruct coords;
	MapClass::Instance->GetCellAt(pCoords)->GetCoords(&coords);

	if(pData->SW_Anim != NULL) {
		coords.Z += pData->SW_AnimHeight;
		AnimClass *placeholder;
		GAME_ALLOC(AnimClass, placeholder, pData->SW_Anim, &coords);
	}

	if(pData->SW_Sound != -1) {
		VocClass::PlayAt(pData->SW_Sound, &coords, NULL);
	}

	if(pData->SW_RadarEvent) {
		RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, *pCoords);
	}

	int TypeIdx = pThis->Type->Type;
	RET_UNLESS(TypeIdx >= FIRST_SW_TYPE);
	return NewSWType::GetNthItem(TypeIdx)->Launch(pThis, pCoords, IsPlayer);
}

void Container<SWTypeExt>::InvalidatePointer(void *ptr) {
	AnnounceInvalidPointer(SWTypeExt::CurrentSWType, ptr);
}

// =============================
// load/save

void Container<SWTypeExt>::Load(SuperWeaponTypeClass *pThis, IStream *pStm) {
	SWTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->SW_Anim);
}

// =============================
// container hooks

DEFINE_HOOK(6CE6F6, SuperWeaponTypeClass_CTOR, 5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	SWTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6CEFE0, SuperWeaponTypeClass_DTOR, 8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);

	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, A)
DEFINE_HOOK_AGAIN(6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(SWTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<SWTypeExt>::SavingObject = pItem;
	Container<SWTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(6CE8BE, SuperWeaponTypeClass_Load_Suffix, 7)
{
	SWTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(6CE8EA, SuperWeaponTypeClass_Save_Suffix, 3)
{
	SWTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(6CEE43, SuperWeaponTypeClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(6CEE50, SuperWeaponTypeClass_LoadFromINI, A)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

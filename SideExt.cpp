#include "SideExt.h"

//Static init
stdext::hash_map<SideClass*, SideExt::Struct> SideExt::Map;
stdext::hash_map<VoxClass*, DynamicVectorClass<SideExt::VoxFileNameStruct> > SideExt::EVAFiles;

//0x679A10
DEFINE_HOOK(679A10, Sides_LoadFromINI, 5)
{
	CCINIClass* pINI = (CCINIClass*)R->get_StackVar32(4);
	if(pINI)
	{
		for(int nSideIndex = 0; nSideIndex < SideClass::Array->get_Count(); nSideIndex++)
		{
			SideClass* pThis = (*SideClass::Array)[nSideIndex];
			char* pID = pThis->get_ID();

			if(_strcmpi(pID, "Civilian") && _strcmpi(pID, "Mutant"))
			{
				//defaults
				if(SideExt::Map.find(pThis) == SideExt::Map.end())
				{
					SideExt::Struct Ext;

					//are these necessary?
					Ext.BaseDefenseCounts.Clear();
					Ext.BaseDefenses.Clear();
					Ext.ParaDrop.Clear();
					Ext.ParaDropNum.Clear();

					if(!_strcmpi(pID, "Nod"))	//Soviets
					{
						for(int i = 0; i < RulesClass::Global()->get_SovietBaseDefenseCounts()->get_Count(); i++)
							Ext.BaseDefenseCounts.AddItem(RulesClass::Global()->get_SovietBaseDefenseCounts()->GetItem(i));

						for(int i = 0; i < RulesClass::Global()->get_SovietBaseDefenses()->get_Count(); i++)
							Ext.BaseDefenses.AddItem(RulesClass::Global()->get_SovietBaseDefenses()->GetItem(i));

						Ext.Crew = RulesClass::Global()->get_SovietCrew();
						Ext.DefaultDisguise = RulesClass::Global()->get_SovietDisguise();
						strcpy(Ext.EVATag, "Russian");
						Ext.LoadTextColor = ColorScheme::Find("SovietLoad");
						
						for(int i = 0; i < RulesClass::Global()->get_SovParaDropInf()->get_Count(); i++)
							Ext.ParaDrop.AddItem((RulesClass::Global()->get_SovParaDropInf()->GetItem(i)));

						for(int i = 0; i < RulesClass::Global()->get_SovParaDropNum()->get_Count(); i++)
							Ext.ParaDropNum.AddItem(RulesClass::Global()->get_SovParaDropNum()->GetItem(i));

//						Ext.PowerPlant = RulesClass::Global()->get_NodRegularPower();
						Ext.SidebarMixFileIndex = 1;
						Ext.SidebarYuriFileNames = false;
						Ext.SurvivorDivisor = RulesClass::Global()->get_SovietSurvivorDivisor();
					}
					else if(!_strcmpi(pID, "ThirdSide"))	//Yuri
					{
						for(int i = 0; i < RulesClass::Global()->get_ThirdBaseDefenseCounts()->get_Count(); i++)
							Ext.BaseDefenseCounts.AddItem(RulesClass::Global()->get_ThirdBaseDefenseCounts()->GetItem(i));

						for(int i = 0; i < RulesClass::Global()->get_ThirdBaseDefenses()->get_Count(); i++)
							Ext.BaseDefenses.AddItem(RulesClass::Global()->get_ThirdBaseDefenses()->GetItem(i));

						Ext.Crew = RulesClass::Global()->get_ThirdCrew();
						Ext.DefaultDisguise = RulesClass::Global()->get_ThirdDisguise();
						strcpy(Ext.EVATag, "Yuri");
						Ext.LoadTextColor = ColorScheme::Find("SovietLoad");
						
						for(int i = 0; i < RulesClass::Global()->get_YuriParaDropInf()->get_Count(); i++)
							Ext.ParaDrop.AddItem(RulesClass::Global()->get_YuriParaDropInf()->GetItem(i));

						for(int i = 0; i < RulesClass::Global()->get_YuriParaDropNum()->get_Count(); i++)
							Ext.ParaDropNum.AddItem(RulesClass::Global()->get_YuriParaDropNum()->GetItem(i));

//						Ext.PowerPlant = RulesClass::Global()->get_ThirdPowerPlant();
						Ext.SidebarMixFileIndex = 1;
						Ext.SidebarYuriFileNames = true;
						Ext.SurvivorDivisor = RulesClass::Global()->get_ThirdSurvivorDivisor();
					}
					else //Allies or any other country
					{
						for(int i = 0; i < RulesClass::Global()->get_AlliedBaseDefenseCounts()->get_Count(); i++)
							Ext.BaseDefenseCounts.AddItem(RulesClass::Global()->get_AlliedBaseDefenseCounts()->GetItem(i));

						for(int i = 0; i < RulesClass::Global()->get_AlliedBaseDefenses()->get_Count(); i++)
							Ext.BaseDefenses.AddItem(RulesClass::Global()->get_AlliedBaseDefenses()->GetItem(i));

						Ext.Crew = RulesClass::Global()->get_AlliedCrew();
						Ext.DefaultDisguise = RulesClass::Global()->get_AlliedDisguise();
						strcpy(Ext.EVATag, "Allied");
						Ext.LoadTextColor = ColorScheme::Find("AlliedLoad");

						for(int i = 0; i < RulesClass::Global()->get_AllyParaDropInf()->get_Count(); i++)
							Ext.ParaDrop.AddItem(RulesClass::Global()->get_AllyParaDropInf()->GetItem(i));

						for(int i = 0; i < RulesClass::Global()->get_AllyParaDropNum()->get_Count(); i++)
							Ext.ParaDropNum.AddItem(RulesClass::Global()->get_AllyParaDropNum()->GetItem(i));

//						Ext.PowerPlant = RulesClass::Global()->get_GDIPowerPlant();
						Ext.SidebarMixFileIndex = 0;
						Ext.SidebarYuriFileNames = false;
						Ext.SurvivorDivisor = RulesClass::Global()->get_AlliedSurvivorDivisor();
					}

					SideExt::Map[pThis] = Ext;
				}
				
				SideExt::Struct* pExt = &SideExt::Map[pThis];
				if(pExt)
				{
					char buffer[BUFLEN];
					char* p = NULL;

					ColorScheme* CS;

					if(pINI->ReadString(pID, "AI.BaseDefenseCounts", "", buffer, BUFLEN))
					{
						pExt->BaseDefenseCounts.Clear();

						for(p = strtok(buffer, ","); p && *p; p = strtok(NULL, ","))
							pExt->BaseDefenseCounts.AddItem(atoi(p));
					}

					if(pINI->ReadString(pID, "AI.BaseDefenses", "", buffer, BUFLEN))
					{
						pExt->BaseDefenses.Clear();

						for(p = strtok(buffer, ","); p && *p; p = strtok(NULL, ","))
							pExt->BaseDefenses.AddItem(BuildingTypeClass::FindOrAllocate(p));
					}

					if(pINI->ReadString(pID, "Crew", "", buffer, 0x80))
						pExt->Crew = InfantryTypeClass::FindOrAllocate(buffer);

					if(pINI->ReadString(pID, "DefaultDisguise", "", buffer, 0x80))
						pExt->DefaultDisguise = InfantryTypeClass::FindOrAllocate(buffer);

					if(pINI->ReadString(pID, "EVA.Tag", "", buffer, 0x20))
						strncpy(pExt->EVATag, buffer, 0x20);

					if(pINI->ReadString(pID, "LoadScreenText.Color", "", buffer, 0x80))
					{
						CS = ColorScheme::Find(buffer);
						if(CS)
							pExt->LoadTextColor = CS;
					}

					if(pINI->ReadString(pID, "ParaDrop.Types", "", buffer, BUFLEN))
					{
						pExt->ParaDrop.Clear();

						for(p = strtok(buffer, ","); p && *p; p = strtok(NULL, ","))
						{
							TechnoTypeClass* pTT = UnitTypeClass::Find(p);
							
							if(!pTT)
								pTT = InfantryTypeClass::Find(p);

							if(pTT)
								pExt->ParaDrop.AddItem(pTT);
						}
					}

					if(pINI->ReadString(pID, "ParaDrop.Num", "", buffer, BUFLEN))
					{
						pExt->ParaDropNum.Clear();

						for(p = strtok(buffer, ","); p && *p; p = strtok(NULL, ","))
							pExt->ParaDropNum.AddItem(atoi(p));
					}

//					if(pINI->ReadString(pID, "AI.PowerPlant", "", buffer, 0x80))
//					pExt->PowerPlant = BuildingTypeClass::FindOrAllocate(buffer);

					pExt->SidebarMixFileIndex = 
						pINI->ReadInteger(pID, "Sidebar.MixFileIndex", pExt->SidebarMixFileIndex);
					pExt->SidebarYuriFileNames = 
						pINI->ReadBool(pID, "Sidebar.YuriFileNames", pExt->SidebarYuriFileNames);
					pExt->SurvivorDivisor = 
						pINI->ReadInteger(pID, "SurvivorDivisor", pExt->SurvivorDivisor);
				}
			}
		}
	}
	return 0;
}

//0x4F8EC6
DEFINE_HOOK(4F8EC6, Sides_BaseUnit, 6)
{
	HouseClass* pThis = (HouseClass*)R->get_ESI();

	if(pThis->get_OwnedBuildings() > 0)
		return 0x4F8F87;	//you survive

	for(int i = 0; i < RulesClass::Global()->get_BaseUnit()->get_Count(); i++)
		if(pThis->get_OwnedUnitTypes()->GetItemCount(RulesClass::Global()->get_BaseUnit()->GetItem(i)->get_ArrayIndex()) > 0)
			return 0x4F8F87;	//you survive	
	
	return 0x4F8F79; //YOU LOSE!!!
}

//0x4F8C97
DEFINE_HOOK(4F8C97, Sides_BuildConst, 6)
{
	HouseClass* pThis = (HouseClass*)R->get_ESI();

	for(int i = 0; i < RulesClass::Global()->get_BuildConst()->get_Count(); i++)
		if(pThis->get_OwnedBuildingTypes1()->GetItemCount(RulesClass::Global()->get_BuildConst()->GetItem(i)->get_ArrayIndex()) > 0)
			return 0x4F8D02;	//"low power"

	return 0x4F8DB1;
}

//0x4F8F54
DEFINE_HOOK(4F8F54, Sides_SlaveMinerCheck, 6)
{
	HouseClass* pThis = (HouseClass*)R->get_ESI();
	int n = R->get_EDI();

	for(int i = 0; i < RulesClass::Global()->get_BuildRefinery()->get_Count(); i++)
		if(RulesClass::Global()->get_BuildRefinery()->GetItem(i)->get_SlavesNumber() > 0)	//new sane way to find a slave miner
			n += pThis->get_OwnedBuildingTypes1()->GetItemCount(RulesClass::Global()->get_BuildRefinery()->GetItem(i)->get_ArrayIndex());

	R->set_EDI(n);
	return 0x4F8F75;
}

//0x505C95
DEFINE_HOOK(505C95, Sides_BaseDefenseCounts, 7)
{
	HouseClass* pThis = (HouseClass*)R->get_EBX();
	int n = R->get_StackVar32(0x80);	//just to be on the safe side, we're not getting it from the House

	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_EAX(pThis->get_AIDifficulty());
		R->set_EDX((DWORD)SideExt::Map[pSide].BaseDefenseCounts.get_Items());
		return 0x505CE6;
	}
	else
		return 0;
}

DWORD SideExt::BaseDefenses(REGISTERS* R, DWORD dwReturnAddress)
{
	HouseTypeClass* pCountry = (HouseTypeClass*)R->get_EAX();

	int n = pCountry->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_EBX((DWORD)&SideExt::Map[pSide].BaseDefenses);
		return dwReturnAddress;
	}
	else
		return 0;
}

//0x507BCA
DEFINE_HOOK(507BCA, Sides_BaseDefenses1, 6)
	{ return SideExt::BaseDefenses(R, 0x507C00); }

//0x507DBA
DEFINE_HOOK(507DBA, Sides_BaseDefenses2, 6)
	{ return SideExt::BaseDefenses(R, 0x507DF0); }

//0x507FAA
DEFINE_HOOK(507FAA, Sides_BaseDefenses3, 6)
	{ return SideExt::BaseDefenses(R, 0x507FE0); }

//0x52267D
DEFINE_HOOK(52267D, Sides_Disguise1, 6)
{
	HouseClass* pHouse = (HouseClass*)R->get_EAX();

	int n = pHouse->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_EAX((DWORD)SideExt::Map[pSide].DefaultDisguise);
		return 0x5226B7;
	}
	else
		return 0;
}

DWORD SideExt::Disguise(REGISTERS* R, DWORD dwReturnAddress, bool bUseESI)
{
	HouseClass* pHouse = (HouseClass*)R->get_EAX();
	InfantryClass* pThis;
	
	if(bUseESI)
		pThis = (InfantryClass*)R->get_ESI();
	else
		pThis = (InfantryClass*)R->get_ECX();

	int n = pHouse->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		pThis->set_Disguise(SideExt::Map[pSide].DefaultDisguise);
		return dwReturnAddress;
	}
	else
		return 0;
}

//0x5227A3
DEFINE_HOOK(5227A3, Sides_Disguise2, 6)
	{ return SideExt::Disguise(R, 0x5227EC, false); }

//0x6F422F
DEFINE_HOOK(6F422F, Sides_Disguise3, 6)
	{ return SideExt::Disguise(R, 0x6F4277, true); }

//0x707D40
DEFINE_HOOK(707D40, Sides_Crew, 6)
{
	HouseClass* pHouse = (HouseClass*)R->get_ECX();

	int n = pHouse->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_ESI((DWORD)SideExt::Map[pSide].Crew);
		return 0x707D81;
	}
	else
		return 0;
}

//0x451358
DEFINE_HOOK(451358, Sides_SurvivorDivisor, 6)
{
	HouseClass* pHouse = (HouseClass*)R->get_EDX();

	int n = pHouse->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_ESI((DWORD)SideExt::Map[pSide].SurvivorDivisor);
		return 0x451391;
	}
	else
		return 0;
}

DWORD SideExt::LoadTextColor(REGISTERS* R, DWORD dwReturnAddress)
{
	int n = R->get_EAX();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		R->set_EAX((DWORD)SideExt::Map[pSide].LoadTextColor);
		return dwReturnAddress;
	}
	else
		return 0;
}

//0x642B36
DEFINE_HOOK(642B36, Sides_LoadTextColor1, 5)
	{ return SideExt::LoadTextColor(R, 0x68CAA9); }

//0x643BB9
DEFINE_HOOK(643BB9, Sides_LoadTextColor2, 5)
	{ return SideExt::LoadTextColor(R, 0x643BEF); }

//0x534FB1
DEFINE_HOOK(534FB1, Sides_MixFileIndex, 5)
{
	int n = R->get_ESI();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
		// original code is 
		// sprtf(mixname, "SIDEC%02dMD.MIX", ESI + 1);
		// it's easier to sub 1 here than to fix the calculation in the orig code
		R->set_ESI(((DWORD)SideExt::Map[pSide].SidebarMixFileIndex) - 1);
	else if(n == 2)
		R->set_ESI(1);

	return 0x534FBB;
}

DWORD SideExt::MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	BYTE* pScenario = (BYTE*)R->get_EAX();	//Scenario, upate this once mapped!
	int n = *((int*)(pScenario + 0x34B8));

	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		if(SideExt::Map[pSide].SidebarYuriFileNames)
			return dwReturnAddress1;
		else
			return dwReturnAddress2;
	}
	else
		return 0;
}

//0x72FA1A
DEFINE_HOOK(72FA1A, Sides_MixFileYuriFiles1, 7)
	{ return SideExt::MixFileYuriFiles(R, 0x72FA23, 0x72FA6A); }

//0x72F370
DEFINE_HOOK(72F370, Sides_MixFileYuriFiles2, 7)
	{ return SideExt::MixFileYuriFiles(R, 0x72F379, 0x72F3A0); }

//0x72FBC3
DEFINE_HOOK(72FBC3, Sides_MixFileYuriFiles3, 5)
	{ return SideExt::MixFileYuriFiles(R, 0x72FBCE, 0x72FBF5); }

//0x6CD3C1
DEFINE_HOOK(6CD3C1, Sides_ParaDrop, 9)
{
	HouseClass* pHouse = ((SuperClass*)R->get_EBX())->get_Owner();

	int n = pHouse->get_SideIndex();
	SideClass* pSide = (*SideClass::Array)[n];

	if(SideExt::Map.find(pSide) != SideExt::Map.end())
	{
		Ares::SendPDPlane(
			pHouse,
			(CellClass*)R->get_EBP(),
			AircraftTypeClass::Array->GetItem(R->get_ESI()),
			&SideExt::Map[pSide].ParaDrop,
			&SideExt::Map[pSide].ParaDropNum);
		
		return 0x6CD500;
	}
	return 0;
}

/*
//0x752F46
EXPORT Sides_LoadVoxFromINI(REGISTERS* R)
{
	VoxClass* pThis = (VoxClass*)R->get_ESI();
	CCINIClass* pINI = (CCINIClass*)R->get_EBP();

	DEBUGLOG("VoxClass::LoadFromINI (%s, pINI = 0x%08X)\n", pThis->get_Name(), pINI);

	DynamicVectorClass<SideExt::VoxFileNameStruct> FileNames;
	SideExt::VoxFileNameStruct vfn;
	char buffer[0x10] = "\0";
	
	for(int i = 0; i < SideClass::Array->get_Count(); i++)
	{
		if(SideExt::Map.find((*SideClass::Array)[i]) != SideExt::Map.end())
		{
			pINI->ReadString(
				pThis->get_Name(),
				SideExt::Map[(*SideClass::Array)[i]].EVATag,
				"",
				buffer,
				0x10);

			strcpy(vfn.FileName, buffer);
		
			FileNames.AddItem(vfn);
		}
		else
		{
			*vfn.FileName = 0;
			FileNames.AddItem(vfn);	//make sure there's an entry for every side
		}
	}

	SideExt::EVAFiles[pThis] = FileNames;
	return 0;	
}
*/

//0x7528E8
DEFINE_HOOK(7528E8, Sides_LoadVoxFile, 7)
{
	VoxClass* pThis = (VoxClass*)R->get_EBP();
	if(SideExt::EVAFiles.find(pThis) != SideExt::EVAFiles.end())
	{
		int nSideIndex = *((int*)0xB1D4C8);

		R->set_EDI((DWORD)SideExt::EVAFiles[pThis][nSideIndex].FileName);
		return 0x752901;
	}

	return 0;
}

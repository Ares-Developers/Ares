#include "BuildingTypeExt.h"
#include "TechnoTypeExt.h"
#include "HouseExt.h"

stdext::hash_map<BuildingTypeClass*, BuildingTypeClassExt::Struct> BuildingTypeClassExt::Map;

//never called - I'll just leave it alone for now
void BuildingTypeClassExt::Struct::Initialize(BuildingTypeClass *pThis)
{
	if(pThis->get_SecretLab())
	{
		DynamicVectorClass<TechnoTypeClass *> *Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretInfantry();
		for(int i = 0; i < Options->get_Count(); ++i)
			Secret_Boons.AddItem(Options->GetItem(i));

		Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretUnits();
		for(int i = 0; i < Options->get_Count(); ++i)
			this->Secret_Boons.AddItem(Options->GetItem(i));

		Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretBuildings();
		for(int i = 0; i < Options->get_Count(); ++i)
			this->Secret_Boons.AddItem(Options->GetItem(i));
	}

	IsInitialized = true;
}

//0x4652ED, 7
DEFINE_HOOK(4652ED, BTExt_Load, 7)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ESI();
	IStream* pStm = (IStream*)R->get_EDI();

	BuildingTypeClassExt::Struct* pExt = &BuildingTypeClassExt::Map[pThis];

	ULONG out;
	pStm->Read(pExt, sizeof(BuildingTypeClassExt::Struct), &out);

	if(pExt->SavegameValidation != BTEXT_VALIDATION)
		Debug::Log("SAVEGAME ERROR: BuildingTypeExt validation is faulty for %s!\n", pThis->get_ID());

	//if there's custom data, read it
	if(pExt->IsCustom && pExt->CustomWidth > 0 && pExt->CustomHeight > 0)
	{
		pExt->CustomData = new CellStruct[pExt->CustomWidth * pExt->CustomHeight + 1];

		pStm->Read(
			pExt->CustomData,
			sizeof(CellStruct) * (pExt->CustomWidth * pExt->CustomHeight + 1),
			&out);

		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(pExt->CustomData);
	}

	return 0;
}

//0x46536E, 3
DEFINE_HOOK(46536E, BTExt_Save, 3)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_StackVar32(0x4);
	IStream* pStm = (IStream*)R->get_StackVar32(0x8);

	BuildingTypeClassExt::Struct* pExt = &BuildingTypeClassExt::Map[pThis];

	if(!pExt->CustomData)
		pExt->IsCustom = true;

	ULONG out;
	pStm->Write(pExt, sizeof(BuildingTypeClassExt::Struct), &out);

	//if there's custom data, write it
	if(pExt->IsCustom)
	{
		pStm->Write(
			pExt->CustomData,
			sizeof(CellStruct) * (pExt->CustomWidth * pExt->CustomHeight + 1),
			&out);
	}

	return 0;
}

//0x464A47, 5
DEFINE_HOOK(464A47, BTExt_LoadFromINI, 5)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_EBP();
	CCINIClass* pINI = (CCINIClass*)R->get_StackVar32(0x36C);

	//Reload the Foundation tag from the ART ini and check whether it is Custom.
	BuildingTypeClassExt::Struct* pData = &BuildingTypeClassExt::Map[pThis];

	if(!pData->IsInitialized)
	{
		pData->Initialize(pThis);
	}

	if(pData->IsCustom)
	{
		//Reset
		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(pData->CustomData);
	}
	else
	{
		char* pID = pThis->get_ImageFile();
		CCINIClass* pArtINI = CCINIClass::INI_Art;

		if(pArtINI)
		{
			char str[0x80]="\0";

			if(pArtINI->ReadString(pID, "Foundation", "", str, 0x80) && !_strcmpi(str,"Custom"))
			{
				//Custom Foundation!
				pData->IsCustom = true;
				pThis->set_Foundation(FOUNDATION_CUSTOM);

				//Load Width and Height
				pData->CustomWidth = pArtINI->ReadInteger(pID, "Foundation.X", 0);
				pData->CustomHeight = pArtINI->ReadInteger(pID, "Foundation.Y", 0);

				//Allocate CellStruct array
				if(pData->CustomData)
					delete pData->CustomData;

				CellStruct* pFoundationData = new CellStruct[pData->CustomWidth * pData->CustomHeight + 1];
				
				pData->CustomData = pFoundationData;
				pThis->set_FoundationData(pFoundationData);

				//Load FoundationData
				CellStruct* pCurrent = pFoundationData;
				char key[0x20];

				for(int i = 0; i < pData->CustomWidth * pData->CustomHeight; i++)
				{
					sprintf(key, "Foundation.%d", i);
					if(pArtINI->ReadString(pID, key, "", str, 0x80) > 0)
					{
						short x, y;
						if(sscanf(str, "%d,%d", &x, &y) == 2)
						{
							pCurrent->X = x;
							pCurrent->Y = y;
							++pCurrent;
						}
					}
					else
					{
						//Set end vector
						pCurrent->X = 0x7FFF;
						pCurrent->Y = 0x7FFF;
						break;
					}
				}
			}
		}
	}


	// secret lab
	char buffer[BUFLEN];
	if(pINI->ReadString(pThis->get_ID(), "SecretLab.PossibleBoons", "", buffer, BUFLEN))
	{
		pData->Secret_Boons.Clear();
		for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ","))
		{
			TechnoTypeClass *pTechno = TechnoTypeClass::Find(cur);
			if(pTechno)
			{
				pData->Secret_Boons.AddItem(pTechno);
			}
		}
	}
	pData->Secret_RecalcOnCapture =
		pINI->ReadBool(pThis->get_ID(), "SecretLab.GenerateOnCapture", pData->Secret_RecalcOnCapture);

	return 0;
}

void BuildingTypeClassExt::UpdateSecretLabOptions(BuildingClass *pThis)
{
	BuildingTypeClass *pType = pThis->get_Type();
	BuildingTypeClassExt::Struct* pData = &BuildingTypeClassExt::Map[pType];
	DEBUGLOG("Secret Lab update for %s\n", pType->get_ID());

	RETZ_UNLESS(pData->Secret_Boons.get_Count() && (!pData->Secret_Placed || pData->Secret_RecalcOnCapture));

	TechnoTypeClass *Result = pType->get_SecretInfantry();
	if(!Result)
	{
		Result = pType->get_SecretUnit();
		if(!Result)
			Result = pType->get_SecretBuilding();
	}
	if(Result)
	{
		pThis->set_SecretProduction(Result);
		return;
	}

	HouseClass *Owner = pThis->get_Owner();
	int OwnerBits = 1 << Owner->get_Type()->get_ArrayIndex();

	DynamicVectorClass<TechnoTypeClass *> Options;
	for(int i = 0; i < pData->Secret_Boons.get_Count(); ++i)
	{
		TechnoTypeClass * Option = pData->Secret_Boons.GetItem(i);
		TechnoTypeClassExt::TechnoTypeClassData* pTech = TechnoTypeClassExt::Ext_p[Option];

		if(pTech->Secret_RequiredHouses & OwnerBits && !(pTech->Secret_ForbiddenHouses & OwnerBits))
		{
			if(!HouseClassExt::RequirementsMet(Owner, Option))
			{
				Options.AddItem(Option);
			}
		}
	}

	if(Options.get_Count() < 1)
	{
		DEBUGLOG("Secret Lab [%s] has no boons applicable to country [%s]!\n",
			pType->get_ID(), Owner->get_Type()->get_ID());
		return;
	}

	int idx = Randomizer::Global()->RandomRanged(0, Options.get_Count() - 1);
	Result = Options[idx];

	DEBUGLOG("Secret Lab rolled %s for %s\n", Result->get_ID(), pType->get_ID());
	pData->Secret_Placed = true;
	pThis->set_SecretProduction(Result);
}

//hook at 0x45EC90
DEFINE_HOOK(45EC90, Foundations_GetFoundationWidth, 6)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	BuildingTypeClassExt::Struct* pData = &BuildingTypeClassExt::Map[pThis];

	if(pData->IsCustom)
	{
		R->set_EAX(pData->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

//hook at 0x45ECE0
DEFINE_HOOK(45ECE0, Foundations_GetFoundationWidth2, 6)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	BuildingTypeClassExt::Struct* pData = &BuildingTypeClassExt::Map[pThis];

	if(pData->IsCustom)
	{
		R->set_EAX(pData->CustomWidth);
		return 0x45ECED;
	}

	return 0;
}

//hook at 0x45ECA0
DEFINE_HOOK(45ECA0, Foundations_GetFoundationHeight, 6)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	BuildingTypeClassExt::Struct* pData = &BuildingTypeClassExt::Map[pThis];

	if(pData->IsCustom)
	{
		bool bIncludeBib = (R->get_StackVar8(0x4) != 0 );
		
		int fH = pData->CustomHeight;
		if(bIncludeBib && pThis->get_Bib())
			++fH;

		R->set_EAX(fH);
		return 0x45ECDA;
	}

	return 0;
}

// old - don't use 448277, 5

// 445F80, 5
DEFINE_HOOK(445F80, BuildingClass_ChangeOwnership, 5)
{
	GET(BuildingClass *, pThis, ESI);
	if(pThis->get_Type()->get_SecretLab())
		BuildingTypeClassExt::UpdateSecretLabOptions(pThis);

	return 0;
}

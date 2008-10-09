#include "BuildingTypeExt.h"
#include "TechnoTypeExt.h"

stdext::hash_map<BuildingTypeClass*, BuildingTypeClassExt::BuildingTypeClassData> BuildingTypeClassExt::Ext_v;

void __stdcall BuildingTypeClassExt::Defaults(BuildingTypeClass* pThis)
{
	if(Ext_v.find(pThis) != Ext_v.end())
	{
		if(Ext_v[pThis].CustomData)
			delete Ext_v[pThis].CustomData;
	}

	Ext_v[pThis].IsCustom = false;
	Ext_v[pThis].CustomWidth = 0;
	Ext_v[pThis].CustomHeight = 0;
	Ext_v[pThis].CustomData = NULL;
	Ext_v[pThis].Secret_Boons.Clear();
}

void __stdcall BuildingTypeClassExt::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	if(Ext_v.find(pThis) != Ext_v.end())
	{
		Defaults(pThis);

		ULONG out;
		pStm->Read(&Ext_v[pThis], sizeof(BuildingTypeClassData), &out);

		//if there's custom data, read it
		if(Ext_v[pThis].IsCustom && Ext_v[pThis].CustomWidth > 0 && Ext_v[pThis].CustomHeight > 0)
		{
			Ext_v[pThis].CustomData = 
				new CellStruct[Ext_v[pThis].CustomWidth * Ext_v[pThis].CustomHeight + 1];

			pStm->Read(
				Ext_v[pThis].CustomData,
				sizeof(CellStruct) * (Ext_v[pThis].CustomWidth * Ext_v[pThis].CustomHeight + 1),
				&out);

			pThis->set_Foundation(FOUNDATION_CUSTOM);
			pThis->set_FoundationData(Ext_v[pThis].CustomData);
		}
	}
}

void __stdcall BuildingTypeClassExt::Save(BuildingTypeClass* pThis, IStream* pStm)
{
	if(Ext_v.find(pThis) != Ext_v.end())
	{
		if(!Ext_v[pThis].CustomData)
			Ext_v[pThis].IsCustom = true;

		ULONG out;
		pStm->Write(&Ext_v[pThis], sizeof(BuildingTypeClassData), &out);

		//if there's custom data, write it
		if(Ext_v[pThis].IsCustom)
		{
			pStm->Write(
				Ext_v[pThis].CustomData,
				sizeof(CellStruct) * (Ext_v[pThis].CustomWidth * Ext_v[pThis].CustomHeight + 1),
				&out);
		}
	}
}

void BuildingTypeClassExt::BuildingTypeClassData::Initialize(BuildingTypeClass *pThis)
{
	DynamicVectorClass<TechnoTypeClass *> *Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretInfantry();
	for(int i = 0; i < Options->get_Count(); ++i)
	{
		this->Secret_Boons.AddItem(Options->GetItem(i));
	}
	Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretUnits();
	for(int i = 0; i < Options->get_Count(); ++i)
	{
		this->Secret_Boons.AddItem(Options->GetItem(i));
	}
	Options = (DynamicVectorClass<TechnoTypeClass *> *)RulesClass::Global()->get_SecretBuildings();
	for(int i = 0; i < Options->get_Count(); ++i)
	{
		this->Secret_Boons.AddItem(Options->GetItem(i));
	}

	this->Data_Initialized = 1;
}

void __stdcall BuildingTypeClassExt::LoadFromINI(BuildingTypeClass* pThis, CCINIClass* pINI)
{
	//Reload the Foundation tag from the ART ini and check whether it is Custom.
	if(Ext_v.find(pThis) == Ext_v.end())
	{
		return;
	}

	BuildingTypeClassData* pData = &Ext_v[pThis];
	if(pData->IsCustom)
	{
		//Reset
		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(Ext_v[pThis].CustomData);
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
	char buffer[1024];
	if(pINI->ReadString(pThis->get_ID(), "SecretLab.PossibleBoons", "", buffer, 1024))
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
	pData->Secret_RecalcOnCapture
		= pINI->ReadBool(pThis->get_ID(), "SecretLab.GenerateOnCapture", pData->Secret_RecalcOnCapture);
}

//hook at 0x45EC90
EXPORT Foundations_GetFoundationWidth(REGISTERS* R)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	if(BuildingTypeClassExt::Ext_v.find(pThis) != BuildingTypeClassExt::Ext_v.end())
	{
		BuildingTypeClassExt::BuildingTypeClassData* pData = &BuildingTypeClassExt::Ext_v[pThis];

		if(pData->IsCustom)
		{
			R->set_EAX(pData->CustomWidth);
			return 0x45EC9D;
		}
	}
	return 0;
}

//hook at 0x45ECE0
EXPORT Foundations_GetFoundationWidth2(REGISTERS* R)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	if(BuildingTypeClassExt::Ext_v.find(pThis) != BuildingTypeClassExt::Ext_v.end())
	{
		BuildingTypeClassExt::BuildingTypeClassData* pData = &BuildingTypeClassExt::Ext_v[pThis];

		if(pData->IsCustom)
		{
			R->set_EAX(pData->CustomWidth);
			return 0x45ECED;
		}
	}
	return 0;
}

//hook at 0x45ECA0
EXPORT Foundations_GetFoundationHeight(REGISTERS* R)
{
	BuildingTypeClass* pThis = (BuildingTypeClass*)R->get_ECX();
	if(BuildingTypeClassExt::Ext_v.find(pThis) != BuildingTypeClassExt::Ext_v.end())
	{
		BuildingTypeClassExt::BuildingTypeClassData* pData = &BuildingTypeClassExt::Ext_v[pThis];

		if(pData->IsCustom)
		{
			bool bIncludeBib = (R->get_StackVar8(0x4) != 0 );
			
			int fH = pData->CustomHeight;
			if(bIncludeBib && pThis->get_Bib())
				++fH;

			R->set_EAX(fH);
			return 0x45ECDA;
		}
	}
	return 0;
}

// old - don't use 448277, 5

// 445F80, 5
EXPORT_FUNC(BuildingClass_ChangeOwnership)
{
	GET(BuildingClass *, pThis, ESI);
	BuildingTypeClass *pType = pThis->get_Type();
	RET_UNLESS(BuildingTypeClassExt::Ext_v.find(pType) != BuildingTypeClassExt::Ext_v.end());
	BuildingTypeClassExt::BuildingTypeClassData* pData = &BuildingTypeClassExt::Ext_v[pType];

	RET_UNLESS(pType->get_SecretLab() && pData->Secret_Boons.get_Count() && pData->Secret_RecalcOnCapture);

	TechnoTypeClass *Result = pType->get_SecretInfantry();
	if(!Result)
	{
		Result = pType->get_SecretUnit();
	}
	if(!Result)
	{
		Result = pType->get_SecretBuilding();
	}
	if(Result)
	{
		pThis->set_SecretProduction(Result);
		return 0;
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
			Options.AddItem(Option);
		}
	}

	if(Options.get_Count() < 1)
	{
		Ares::Log("Secret Lab [%s] has no boons applicable to country [%s]!\n",
			pType->get_ID(), Owner->get_Type()->get_ID());
		return 0;
	}

	int idx = Randomizer::Global()->RandomRanged(0, Options.get_Count() - 1);
	Result = Options[idx];

	pThis->set_SecretProduction(Result);
	return 0;
}

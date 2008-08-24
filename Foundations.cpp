#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

//Allows custom foundations for buildings

#include "Foundations.h"

//Static init
stdext::hash_map<BuildingTypeClass*,Foundations::CustomFoundationStruct> Foundations::CustomFoundation;

void __stdcall Foundations::Defaults(BuildingTypeClass* pThis)
{
	if(CustomFoundation.find(pThis) != CustomFoundation.end())
	{
		if(CustomFoundation[pThis].CustomData)
			delete CustomFoundation[pThis].CustomData;
	}

	CustomFoundation[pThis].IsCustom = false;
	CustomFoundation[pThis].CustomWidth = 0;
	CustomFoundation[pThis].CustomHeight = 0;
	CustomFoundation[pThis].CustomData = NULL;
}

void __stdcall Foundations::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	if(CustomFoundation.find(pThis) != CustomFoundation.end())
	{
		Defaults(pThis);

		ULONG out;
		pStm->Read(&CustomFoundation[pThis], sizeof(CustomFoundationStruct), &out);

		//if there's custom data, read it
		if(CustomFoundation[pThis].IsCustom && CustomFoundation[pThis].CustomWidth > 0 && CustomFoundation[pThis].CustomHeight > 0)
		{
			CustomFoundation[pThis].CustomData = 
				new CellStruct[CustomFoundation[pThis].CustomWidth * CustomFoundation[pThis].CustomHeight + 1];

			pStm->Read(
				CustomFoundation[pThis].CustomData,
				sizeof(CellStruct) * (CustomFoundation[pThis].CustomWidth * CustomFoundation[pThis].CustomHeight + 1),
				&out);

			pThis->set_Foundation(FOUNDATION_CUSTOM);
			pThis->set_FoundationData(CustomFoundation[pThis].CustomData);
		}
	}
}

void __stdcall Foundations::Save(BuildingTypeClass* pThis, IStream* pStm)
{
	if(CustomFoundation.find(pThis) != CustomFoundation.end())
	{
		if(!CustomFoundation[pThis].CustomData)
			CustomFoundation[pThis].IsCustom = true;

		ULONG out;
		pStm->Write(&CustomFoundation[pThis], sizeof(CustomFoundationStruct), &out);

		//if there's custom data, write it
		if(CustomFoundation[pThis].IsCustom)
		{
			pStm->Write(
				CustomFoundation[pThis].CustomData,
				sizeof(CellStruct) * (CustomFoundation[pThis].CustomWidth * CustomFoundation[pThis].CustomHeight + 1),
				&out);
		}
	}
}

void __stdcall Foundations::LoadFromINI(BuildingTypeClass* pThis, CCINIClass* pINI)
{
	//Reload the Foundation tag from the ART ini and check whether it is Custom.
	if(CustomFoundation.find(pThis) == CustomFoundation.end())
	{
		return;
	}

	if(CustomFoundation[pThis].IsCustom)
	{
		//Reset
		pThis->set_Foundation(FOUNDATION_CUSTOM);
		pThis->set_FoundationData(CustomFoundation[pThis].CustomData);
	}
	else
	{
		char* pID = pThis->get_ImageFile();
		CCINIClass* pArtINI=CCINIClass::INI_Art;

		if(pArtINI)
		{
			char str[0x80]="\0";

			if(pArtINI->ReadString(pID, "Foundation", "", str, 0x80) && !_strcmpi(str,"Custom"))
			{
				//Custom Foundation!
				CustomFoundationStruct* pData = &CustomFoundation[pThis];

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
}

//hook at 0x45EC90
EXPORT Foundations_GetFoundationWidth(REGISTERS* R)
{
	BuildingTypeClass* pThis=(BuildingTypeClass*)R->get_ECX();
	if(Foundations::CustomFoundation.find(pThis)!=Foundations::CustomFoundation.end())
	{
		Foundations::CustomFoundationStruct* pData=&Foundations::CustomFoundation[pThis];

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
	BuildingTypeClass* pThis=(BuildingTypeClass*)R->get_ECX();
	if(Foundations::CustomFoundation.find(pThis)!=Foundations::CustomFoundation.end())
	{
		Foundations::CustomFoundationStruct* pData=&Foundations::CustomFoundation[pThis];

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
	BuildingTypeClass* pThis=(BuildingTypeClass*)R->get_ECX();
	if(Foundations::CustomFoundation.find(pThis)!=Foundations::CustomFoundation.end())
	{
		Foundations::CustomFoundationStruct* pData=&Foundations::CustomFoundation[pThis];

		if(pData->IsCustom)
		{
			bool bIncludeBib=(R->get_StackVar8(0x4)!=0);
			
			int fH=pData->CustomHeight;
			if(bIncludeBib && pThis->get_Bib())fH+=1;

			R->set_EAX(fH);
			return 0x45ECDA;
		}
	}
	return 0;
}

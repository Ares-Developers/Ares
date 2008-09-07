#include <YRPP.h>
#include "TechnoTypeExt.h"
#include "Sides.h"

EXT_P_DECLARE(TechnoTypeClass);

EXT_CTOR(TechnoTypeClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Survivors_Pilots.SetCapacity(1, NULL);

		pData->Survivors_PassengersEscape = 0;
		pData->Survivors_PilotChance = 0;
		pData->Survivors_PassengerChance = 0;
		pData->Survivors_Pilots[0] = NULL;

		pData->Data_Initialized = 0;

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);

		for(int i = 0; i < Ext_p[pThis]->Survivors_Pilots.get_Count(); ++i)
		{
			SWIZZLE(Ext_p[pThis]->Survivors_Pilots[i]);
		}
	}
}

EXT_SAVE(TechnoTypeClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void TechnoTypeClassExt::TechnoTypeClassData::Initialize(TechnoTypeClass *pThis)
{
	this->Survivors_Pilots.SetCapacity(SideClass::Array->get_Count(), NULL);

	this->Survivors_PassengersEscape = pThis->get_Crewed();
	this->Survivors_PilotChance = (int)RulesClass::Global()->get_CrewEscape() * 100;
	this->Survivors_PassengerChance = (int)RulesClass::Global()->get_CrewEscape() * 100;

	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
	{
		this->Survivors_Pilots[i] = Sides::SideExt[SideClass::Array->GetItem(i)].Crew;
	}

	this->Data_Initialized = 1;
}

EXT_LOAD_INI(TechnoTypeClass)
{
	const char * section = pThis->get_ID();
	if(!CONTAINS(Ext_p, pThis) || !pINI->GetSection(section))
	{
		return;
	}

	ExtData *pData = Ext_p[pThis];
	if(!pData->Data_Initialized)
	{
		pData->Initialize(pThis);
	}

	pData->Survivors_Pilots.SetCapacity(SideClass::Array->get_Count(), NULL);

	pData->Survivors_PassengersEscape = pINI->ReadBool(section, "Survivor.PassengersSurvive", pData->Survivors_PassengersEscape);
	pData->Survivors_PilotChance = pINI->ReadInteger(section, "Survivor.PilotChance", pData->Survivors_PilotChance);
	pData->Survivors_PassengerChance = pINI->ReadInteger(section, "Survivor.PassengerChance", pData->Survivors_PassengerChance);

	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
	{
		char buffer[256];
		char flag[256];
		sprintf(flag, "Survivor.Side%d", i);
		PARSE_INFANTRY(flag, pData->Survivors_Pilots[i]);
	}
}


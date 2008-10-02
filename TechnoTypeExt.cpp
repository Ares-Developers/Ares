#include <YRPP.h>
#include "TechnoTypeExt.h"
#include "Sides.h"

EXT_P_DEFINE(TechnoTypeClass);

EXT_CTOR(TechnoTypeClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->Survivors_Pilots.SetCapacity(1, NULL);

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

	this->Survivors_PilotChance = (int)RulesClass::Global()->get_CrewEscape() * 100;
	this->Survivors_PassengerChance = (int)RulesClass::Global()->get_CrewEscape() * 100;

	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
	{
		this->Survivors_Pilots[i] = Sides::SideExt[SideClass::Array->GetItem(i)].Crew;
	}

//	this->Cameo_Interval = 300;
//	this->Cameo_CurrentFrame = 0;
//	this->Cameo_Timer.Stop();

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

	pData->Survivors_PilotChance = pINI->ReadInteger(section, "Survivor.PilotChance", pData->Survivors_PilotChance);
	pData->Survivors_PassengerChance = pINI->ReadInteger(section, "Survivor.PassengerChance", pData->Survivors_PassengerChance);

	char buffer[256];
	char flag[256];
	for(int i = 0; i < SideClass::Array->get_Count(); ++i)
	{
		sprintf(flag, "Survivor.Side%d", i);
		PARSE_INFANTRY(flag, pData->Survivors_Pilots[i]);
	}

//	pData->Cameo_Interval = 
//		CCINIClass::INI_Art->ReadInteger(section, "Cameo.Interval", pData->Cameo_Interval);
}

/*
 * broken - stupid game only redraws when mouse is moved over a cameo edge or sw tab is blinking fsds
// 6A9A2A, 6
EXPORT_FUNC(TabCameoListClass_Draw)
{
	TechnoTypeClass * T = (TechnoTypeClass *)R->get_StackVar32(0x6C);
	RET_UNLESS(CONTAINS(TechnoTypeClassExt::Ext_p, T));
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[T];

	else if(pData->Cameo_Interval && !(Unsorted::CurrentFrame % pData->Cameo_Interval))
	{
		pData->Cameo_CurrentFrame = 
			pData->Cameo_CurrentFrame < T->GetCameo()->Frames - 1
				? pData->Cameo_CurrentFrame + 1
				: 0;
		R->set_StackVar32(4, pData->Cameo_CurrentFrame);
		pData->Cameo_Timer.StartIfEmpty();
		if(!strcmp("BFRT", T->get_ID()))
		{
			wchar_t msg[0x40] = L"\0";
			wsprintf(msg, L"BFRT cameo rolled");
			MessageListClass::PrintMessage(msg);
		}
	}
	return 0;
}
*/


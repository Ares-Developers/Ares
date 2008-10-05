#include <YRPP.h>
#include "RulesExt.h"
#include "Prerequisites.h"

RulesClassExt::RulesClassData* RulesClassExt::Data;

EXT_CTOR(RulesClass)
{
	RulesClassExt::Global()->Data_Initialized = 0;
}

EXT_DTOR(RulesClass)
{
	delete RulesClassExt::Global();
}

EXT_LOAD(RulesClass)
{
	ULONG out;
	pStm->Read(RulesClassExt::Global(), sizeof(RulesClassExt::RulesClassData), &out);
}

EXT_SAVE(RulesClass)
{
	ULONG out;
	pStm->Write(RulesClassExt::Global(), sizeof(RulesClassExt::RulesClassData), &out);
}

void RulesClassExt::RulesClassData::Initialize()
{
	RulesClassExt::RulesClassData *pData = RulesClassExt::Global();
	RulesClass * pRules = RulesClass::Global();

	GenericPrerequisite::FindOrAllocate("POWER");
	GenericPrerequisite::FindOrAllocate("FACTORY");
	GenericPrerequisite::FindOrAllocate("BARRACKS");
	GenericPrerequisite::FindOrAllocate("RADAR");
	GenericPrerequisite::FindOrAllocate("TECH");
	GenericPrerequisite::FindOrAllocate("PROC");
	this->Data_Initialized = 1;
}

EXT_LOAD_INI(RulesClass)
{
	RulesClassExt::RulesClassData *pData = RulesClassExt::Global();
	if(!pData->Data_Initialized)
	{
		pData->Initialize();
	}
}

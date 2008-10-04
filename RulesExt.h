#ifndef RULES_EXT_H
#define RULES_EXT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

#include <hash_map>

class RulesClassExt
{
	public:

	struct RulesClassData
	{
		bool Data_Initialized;
		void Initialize();
	};

	private:
		static RulesClassData* Data;

	static RulesClassData* Global()
	{
		if(!Data)
		{
			Data = new RulesClassData();
		}
		return Data;
	};

	EXT_FUNCS(RulesClass);
	EXT_INI_FUNCS(RulesClass);

};

#endif

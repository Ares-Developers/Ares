#ifndef TECHNOTYPE_EXT_H
#define TECHNOTYPE_EXT_H

#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class TechnoTypeClassExt
{
	public:
	struct TechnoTypeClassData
	{
		bool   Type_IsCustom;
	};

	EXT_P_DEFINE(TechnoTypeClass);
};

#endif

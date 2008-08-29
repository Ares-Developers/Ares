#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class WarheadTypeClassExt
{
	public:
	struct WarheadTypeClassData
	{
		bool MindControl_Permanent;

		int EMP_Duration;

		int IC_Duration;
		AnimTypeClass *IC_Anim;
	};

	EXT_P_DEFINE(WarheadTypeClass);
};

#endif

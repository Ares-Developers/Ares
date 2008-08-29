#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class WarheadTypeClassExt
{
	public:
	struct WarheadTypeClassData
	{
		bool IsCustom;
		bool MindControl_Permanent;
		int EMP_Duration;
	};

	EXT_P_DEFINE(WarheadTypeClass);
};
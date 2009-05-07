#include "Debug.h"

DEFINE_HOOK(45E8A5, BuildingTypeClass_CreateBuilding, 6)
{
	Debug::Log("!malloc @ %s!", __FUNCTION__);

	return 0;
}

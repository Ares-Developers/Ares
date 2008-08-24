#include <YRPP.h>
#include <YRCallbacks.h>

EXPORT_FUNC(RulesClass_Addition)
{
	CCINIClass *pINI = (CCINIClass *)R->get_ESI();

	ARRAY_ITERATE(WarheadTypeClass, pINI);
//	callbacks not defined yet
//	ARRAY_ITERATE(BulletTypeClass, pINI);
	ARRAY_ITERATE(WeaponTypeClass, pINI);
	return 0;
}
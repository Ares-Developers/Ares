#include <YRPP.h>
#include <YRCallbacks.h>

EXPORT_FUNC(RulesClass_Addition)
{
	CCINIClass *pINI = (CCINIClass *)R->get_ESI();

	ARRAY_ITERATE(WarheadTypeClass, pINI);
//	callbacks not defined yet
//	ARRAY_ITERATE(BulletTypeClass, pINI);
	ARRAY_ITERATE(WeaponTypeClass, pINI);

	ARRAY_ITERATE(SuperWeaponTypeClass, pINI);

	// HAXXX, stop looking at me like that
	if(TechnoTypeClassCallback::LoadFromINI)
	{
		for(int i = 0; i < TechnoTypeClass::Array->get_Count(); ++i)
		{
			TechnoTypeClassCallback::LoadFromINI((TechnoTypeClass *)(TechnoTypeClass::Array->GetItem(i)), pINI);
		}
	}

	return 0;
}
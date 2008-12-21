#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <YRCallbacks.h>

DEFINE_HOOK(668F6A, RulesClass_Addition, 5)
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
			TechnoTypeClassCallback::LoadFromINI((TechnoTypeClass *)TechnoTypeClass::Array->GetItem(i), pINI);
		}
	}

	return 0;
}

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
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

//668D86, 5
// adding a [WeaponTypes] - no more WEEDGUY hax
EXPORT_FUNC(RulesClass_NewLists)
{
	GET(CCINIClass *, pINI, ESI);

	char buffer[0x20];

	const char section[] = "WeaponTypes";

	int len = pINI->GetKeyCount(section);
	for(int i = 0; i < len; ++i)
	{
		const char *key = pINI->GetKeyName(section, i);
		if(pINI->ReadString(section, key, "", buffer, 0x20) > 0)
		{
			WeaponTypeClass::FindOrAllocate(buffer);
		}
	}

	return 0;
}


#include "Body.h"

DEFINE_HOOK(6FE709, TechnoClass_Fire_BallisticScatter1, 6)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExt::ExtMap.Find(pProjectile);

	// defaults for FlakScatter && !Inviso
	int default = RulesClass::Instance->BallisticScatter;
	int min = pExt->GetBallisticScatterMin(0);
	int max = pExt->GetBallisticScatterMax(default);
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE71C;
}

DEFINE_HOOK(6FE7FE, TechnoClass_Fire_BallisticScatter2, 5)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExt::ExtMap.Find(pProjectile);

	// defaults for !FlakScatter || Inviso
	int default = RulesClass::Instance->BallisticScatter;
	int min = pExt->GetBallisticScatterMin(default / 2);
	int max = pExt->GetBallisticScatterMax(default);
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE821;
}

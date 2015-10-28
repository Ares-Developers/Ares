#include "../Ext/Rules/Body.h"

/***
 * the following hooks replace the original checks that disable certain visual
 * effects when the frame rate drops below a certain limit. the issue with them
 * was that they only take into account the settings from the rulesmd.ini, but
 * ignore the game speed setting. that means if the frame rate is supposed to
 * be 20 frames or less (DetailMinFrameRateNormal=15, DetailBufferZoneWidth=5),
 * then the effects are still disabled. lasers draw as ugly lines, non-damaging
 * particles don't render, spotlights aren't created, ...
 ***/

DEFINE_HOOK(48A634, FlashbangWarheadAt_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x48A64Au : 0x48A641u;
}

DEFINE_HOOK(5FF86E, SpotlightClass_Draw_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x5FF87Fu : 0x5FFF77u;
}

DEFINE_HOOK(422FCC, AnimClass_Draw_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x422FECu : 0x422FD9u;
}

DEFINE_HOOK(550BCA, LaserDrawClass_Draw_InHouseColor_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x550BD7u : 0x550BE5u;
}

DEFINE_HOOK(62CEC9, ParticleClass_Draw_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x62CEEAu : 0x62CED6u;
}

DEFINE_HOOK(6D7847, TacticalClass_DrawPixelEffects_Details, 5)
{
	auto const details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x6D7858u : 0x6D7BF2u;
}

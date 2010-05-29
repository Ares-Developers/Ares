#include "RadTypes.h"

DynamicVectorClass<RadType*> Enumerable<RadType>::Array;

// pretty nice, eh
const char * Enumerable<RadType>::GetMainSection()
{
	return "RadiationTypes";
}

void RadType::LoadFromINI(CCINIClass *pINI)
{
	const char *section = this->Name;

	INI_EX exINI(pINI);

	this->WH.Parse(&exINI, section, "Warhead");
	this->Color.Read(&exINI, section, "Color");
	this->Duration_Multiple.Read(&exINI, section, "DurationMultiple");
	this->Application_Delay.Read(&exINI, section, "ApplicationDelay");
	this->Level_Max.Read(&exINI, section, "LevelMax");
	this->Level_Delay.Read(&exINI, section, "LevelDelay");
	this->Light_Delay.Read(&exINI, section, "LightDelay");
	this->Level_Factor.Read(&exINI, section, "LevelFactor");
	this->Light_Factor.Read(&exINI, section, "LightFactor");
	this->Tint_Factor.Read(&exINI, section, "TintFactor");
}

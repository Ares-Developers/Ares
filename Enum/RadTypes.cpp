#include "RadTypes.h"

DynamicVectorClass<RadType*> Enumerable<RadType>::Array;

// pretty nice, eh
const char * Enumerable<RadType>::GetMainSection()
{
	return "RadiationTypes";
}

void RadType::LoadFromINI(CCINIClass *pINI)
{
	ColorStruct tmpColor;

	const char *section = this->Name;

	PARSE_BUF();

	PARSE_WH("Warhead", this->WH);
	PARSE_COLOR("Color", this->Color, tmpColor);
	this->Duration_Multiple = pINI->ReadInteger(section, "DurationMultiple", this->Duration_Multiple);
	this->Application_Delay = pINI->ReadInteger(section, "ApplicationDelay", this->Application_Delay);
	this->Level_Max    = pINI->ReadInteger(section, "LevelMax", this->Level_Max);
	this->Level_Delay  = pINI->ReadInteger(section, "LevelDelay", this->Level_Delay);
	this->Light_Delay  = pINI->ReadInteger(section, "LightDelay", this->Light_Delay);
	this->Level_Factor = pINI->ReadDouble(section, "LevelFactor", this->Level_Factor);
	this->Light_Factor = pINI->ReadDouble(section, "LightFactor", this->Light_Factor);
	this->Tint_Factor  = pINI->ReadDouble(section, "TintFactor", this->Tint_Factor);
}

#ifndef RAD_TYPES_H
#define RAD_TYPES_H

#include <Helpers\Macro.h>
#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <RadSiteClass.h>
#include <RulesClass.h>

#include "_Enumerator.hpp"

#ifdef DEBUGBUILD
#include "..\Misc\Debug.h"
#endif

class RadType;

class RadType : public Enumerable<RadType>
{
public:
	virtual void LoadFromINI(CCINIClass *pINI);

/*
	template <typename T2>
	static const char * GetMainSection();

	virtual const char * GetMainSection()
	{
		return "RadiationTypes";
	};
*/
	RadType(const char *Title)
	{
		strncpy(this->Name, Title, 32);
		Array.AddItem(this);

		RulesClass *Rules = RulesClass::Global();
		this->WH = Rules->RadSiteWarhead;
		this->Color = *Rules->get_RadColor();
		this->Duration_Multiple = Rules->RadDurationMultiple;
		this->Application_Delay = Rules->RadApplicationDelay;
		this->Level_Max = Rules->RadLevelMax;
		this->Level_Delay = Rules->RadLevelDelay;
		this->Light_Delay = Rules->RadLightDelay;
		this->Level_Factor = Rules->RadLevelFactor;
		this->Light_Factor = Rules->RadLightFactor;
		this->Tint_Factor = Rules->RadTintFactor;
	}

	virtual ~RadType()
	{
		Array.RemoveItem(Array.FindItemIndex(this));
	}

	WarheadTypeClass *WH;
	ColorStruct Color;
	int Duration_Multiple;
	int Application_Delay;
	int Level_Max;
	int Level_Delay;
	int Light_Delay;
	double Level_Factor;
	double Light_Factor;
	double Tint_Factor;
};

#endif

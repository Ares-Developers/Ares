#ifndef RAD_TYPES_H
#define RAD_TYPES_H

#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <RadSiteClass.h>
#include <RulesClass.h>

#include "_Enumerator.hpp"
#include "../Ares.CRT.h"
#include "../Utilities/Template.h"

#ifdef DEBUGBUILD
#include "../Misc/Debug.h"
#endif

class RadType;

class RadType : public Enumerable<RadType>
{
public:

	Customizable<WarheadTypeClass *> WH;
	Customizable<ColorStruct> Color;
	Customizable<int> Duration_Multiple;
	Customizable<int> Application_Delay;
	Customizable<int> Level_Max;
	Customizable<int> Level_Delay;
	Customizable<int> Light_Delay;
	Customizable<double> Level_Factor;
	Customizable<double> Light_Factor;
	Customizable<double> Tint_Factor;

	virtual void LoadFromINI(CCINIClass *pINI);

	RadType(const char *Title) :
		WH(&RulesClass::Instance->RadSiteWarhead),
		Color(&RulesClass::Instance->RadColor),
		Duration_Multiple(&RulesClass::Instance->RadDurationMultiple),
		Application_Delay(&RulesClass::Instance->RadApplicationDelay),
		Level_Max(&RulesClass::Instance->RadLevelMax),
		Level_Delay(&RulesClass::Instance->RadLevelDelay),
		Light_Delay(&RulesClass::Instance->RadLightDelay),
		Level_Factor(&RulesClass::Instance->RadLevelFactor),
		Light_Factor(&RulesClass::Instance->RadLightFactor),
		Tint_Factor(&RulesClass::Instance->RadTintFactor)
	{
		AresCRT::strCopy(this->Name, Title);
		Array.AddItem(this);
	}

	virtual ~RadType()
	{
		RadType * placeholder = this;
		Array.RemoveItem(Array.FindItemIndex(&placeholder));
	}
};

#endif

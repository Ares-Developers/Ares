#ifndef RAD_TYPES_H
#define RAD_TYPES_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <RadSiteClass.h>
#include <RulesClass.h>

#include "Enumerator.h"

#ifdef DEBUGBUILD
#include "Debug.h"
#endif

class RadType;
//template<RadType>
//	template <RadType>

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
		this->WH = Rules->get_RadSiteWarhead();
		this->Color = *Rules->get_RadColor();
		this->Duration_Multiple = Rules->get_RadDurationMultiple();
		this->Application_Delay = Rules->get_RadApplicationDelay();
		this->Level_Max = Rules->get_RadLevelMax();
		this->Level_Delay = Rules->get_RadLevelDelay();
		this->Light_Delay = Rules->get_RadLightDelay();
		this->Level_Factor = Rules->get_RadLevelFactor();
		this->Light_Factor = Rules->get_RadLightFactor();
		this->Tint_Factor = Rules->get_RadTintFactor();
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

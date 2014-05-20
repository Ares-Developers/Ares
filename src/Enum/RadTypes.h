#ifndef RAD_TYPES_H
#define RAD_TYPES_H

#include <WarheadTypeClass.h>
#include <RadSiteClass.h>
#include <RulesClass.h>

#include "_Enumerator.hpp"
#include "../Utilities/Template.h"

class RadType;
class CCINIClass;

class RadType : public Enumerable<RadType>
{
private:
	Nullable<WarheadTypeClass *> WH;
	Nullable<ColorStruct> Color;
	Nullable<int> Duration_Multiple;
	Nullable<int> Application_Delay;
	Nullable<int> Level_Max;
	Nullable<int> Level_Delay;
	Nullable<int> Light_Delay;
	Nullable<double> Level_Factor;
	Nullable<double> Light_Factor;
	Nullable<double> Tint_Factor;

public:
	RadType(const char *Title) : Enumerable<RadType>(Title),
		WH(),
		Color(),
		Duration_Multiple(),
		Application_Delay(),
		Level_Max(),
		Level_Delay(),
		Light_Delay(),
		Level_Factor(),
		Light_Factor(),
		Tint_Factor()
	{ }

	virtual ~RadType() override = default;

	virtual void LoadFromINI(CCINIClass *pINI) override;

	WarheadTypeClass* GetWarhead() const {
		return this->WH.Get(RulesClass::Instance->RadSiteWarhead);
	}

	const ColorStruct& GetColor() const {
		return *this->Color.GetEx(&RulesClass::Instance->RadColor);
	}

	int GetDurationMultiple() const {
		return this->Duration_Multiple.Get(RulesClass::Instance->RadDurationMultiple);
	}

	int GetApplicationDelay() const {
		return this->Application_Delay.Get(RulesClass::Instance->RadApplicationDelay);
	}

	int GetLevelMax() const {
		return this->Level_Max.Get(RulesClass::Instance->RadLevelMax);
	}

	int GetLevelDelay() const {
		return this->Level_Delay.Get(RulesClass::Instance->RadLevelDelay);
	}

	int GetLightDelay() const {
		return this->Light_Delay.Get(RulesClass::Instance->RadLightDelay);
	}

	double GetLevelFactor() const {
		return this->Level_Factor.Get(RulesClass::Instance->RadLevelFactor);
	}

	double GetLightFactor() const {
		return this->Light_Factor.Get(RulesClass::Instance->RadLightFactor);
	}

	double GetTintFactor() const {
		return this->Tint_Factor.Get(RulesClass::Instance->RadTintFactor);
	}
};

#endif

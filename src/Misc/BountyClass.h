#ifndef BOUNTYCLASS_H_
#define BOUNTYCLASS_H_

#include "../Utilities/Template.h"

class BountyClass {
public:

	Nullable<bool> Message;
	Nullable<double> Modifier;
	Nullable<double> FriendlyModifier;
	// #1523 also Money Conversion -> Pillage
	Nullable<bool> Pillager;
	Nullable<double> CostMultiplier;
	Nullable<double> PillageMultiplier;

	BountyClass(
		bool Msg = false,
		double Mod = 0,
		double FriendlyMod = 0,
		bool IsPillager = false,
		double CostMult = 1,
		double PillageMult = 1
		)
		
	{
		Message.SetDefault(Msg);
		Modifier.SetDefault (Mod);
		FriendlyModifier.SetDefault (FriendlyMod);
		Pillager.SetDefault (IsPillager);
		CostMultiplier.SetDefault (CostMult);
		PillageMultiplier.SetDefault (PillageMult);
	};

	void Read(INI_EX *exINI, const char * section);

	static void CalculateBounty(TechnoClass* Attacker, TechnoClass* Victim, int PillageDamage);
	static void BountyMessageOutput(TechnoClass* Messager);
	
};

#endif

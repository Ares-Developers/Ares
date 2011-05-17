#ifndef BOUNTYCLASS_H_
#define BOUNTYCLASS_H_

#include "../Utilities/Template.h"

class BountyClass {
public:

	Nullable<bool> Message;
	Nullable<bool> FriendlyMessage;
	Nullable<double> Modifier;
	Nullable<double> FriendlyModifier;
	// #1523 also Money Conversion -> Pillage
	Nullable<bool> Pillager;
	Nullable<double> CostMultiplier;
	Nullable<double> PillageMultiplier;

	BountyClass(bool Msg = false, bool FriendlyMsg = false,
			double Mod = 0, double FriendlyMod = 0, bool IsPillager = false,
			double CostMult = 0, double PillageMult = 0):
		Message (Msg),
		FriendlyMessage (FriendlyMsg),
		Modifier (Mod),
		FriendlyModifier (FriendlyMod),
		Pillager (IsPillager),
		CostMultiplier (CostMult),
		PillageMultiplier (PillageMult)
	{
	};

	void Read(INI_EX *exINI, const char * section) {
		this->Message.Read(exINI, section, "Bounty.Message");
		this->FriendlyMessage.Read(exINI, section, "Bounty.FriendlyMessage");
		this->Modifier.Read(exINI, section, "Bounty.Modifier");
		this->FriendlyModifier.Read(exINI, section, "Bounty.FriendlyModifier");
		this->CostMultiplier.Read(exINI, section, "Bounty.CostMultiplier");
		// #1523 Money-Conversion per applied Damage... tag will be Bounty.Pillager
		this->Pillager.Read(exINI, section, "Bounty.Pillager");
		this->PillageMultiplier.Read(exINI, section, "Bounty.PillageMultiplier");
	}
};

#endif

#ifndef BUILDING_PRISM_FWD_H
#define BUILDING_PRISM_FWD_H

#include <BuildingClass.h>

namespace BuildingExtras {
	class cPrismForwarding {
		public:
		DynamicVectorClass<BuildingClass*> Senders;		//the prism towers that are forwarding to this one
		BuildingClass* SupportTarget;				//what tower am I sending to?
		int PrismChargeDelay;					//current delay charge
		double ModifierReserve;					//current modifier reservoir
		int DamageReserve;					//current flat reservoir

		// constructor
		cPrismForwarding() : SupportTarget(NULL), PrismChargeDelay(0), ModifierReserve(0.0), DamageReserve(0) {
			this->Senders.Clear();
		};
	};
};


#endif

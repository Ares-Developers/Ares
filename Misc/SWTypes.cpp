#include "SWTypes.h"
#include "SWTypes/SonarPulse.h"

DynamicVectorClass<NewSWType *> NewSWType::Array;

void NewSWType::Init()
{
	new SW_SonarPulse();
	new SW_GenericWarhead();
}

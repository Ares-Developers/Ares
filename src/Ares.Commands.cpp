#include "Ares.h"

#include "Commands/Commands.h"

// include other commands like this
#include "Commands/AIControl.h"
#include "Commands/MapSnapshot.h"
//include "Commands/FrameByFrame.h"
#include "Commands/AIBasePlan.h"
#include "Commands/DumpTypes.h"
#include "Commands/DumpMemory.h"
//#include "Commands/Debugging.h"
//include "Commands/Logging.h"
#include "Commands/FPSCounter.h"
#include "Commands/TogglePower.h"

void Ares::RegisterCommands()
{
	if(bAllowAIControl) {
		MakeCommand<AIControlCommandClass>();
	}
	MakeCommand<MapSnapshotCommandClass>();
	//MakeCommand<TestSomethingCommandClass>();
	MakeCommand<DumperTypesCommandClass>();
	MakeCommand<MemoryDumperCommandClass>();
	//MakeCommand<DebuggingCommandClass>();
	MakeCommand<AIBasePlanCommandClass>();
	MakeCommand<FPSCounterCommandClass>();
	MakeCommand<TogglePowerCommandClass>();
}

DEFINE_HOOK(533058, CommandClassCallback_Register, 7)
{
	Ares::RegisterCommands();

	DWORD* D = GameCreate<DWORD>();
	R->EAX(D);	//Allocate SetUnitTabCommandClass
	return 0x533062;
}

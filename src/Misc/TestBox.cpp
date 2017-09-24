#include "TestBox.h"

#include <BuildingClass.h>
#include <AircraftClass.h>
#include <Helpers/Template.h>

void TestScaffold::FloatToIntTest() {
	Debug::Log("Starting FloatToInt test\n");

	for(int i = 0; i < 10; ++i) {
		for(double f = -5.75; f <= 5.75; f += 0.05) {
			Debug::Log("%lf -> %I64d\n", f, Game::F2I64(f));
		}
	}

	Debug::Log("Finished FloatToInt test\n");
}

void TestScaffold::GameCastTest() {
	Debug::Log("Starting g_c test\n");
	ObjectClass *O = nullptr; // doesn't matter how you got the pointer, as long as it's valid... check for NULL yourselves
	
	if(BuildingClass *B = specific_cast<BuildingClass *>(O)) {
		Debug::Log("Casted to Building - correct\n");
	}

	if(AircraftClass *A = specific_cast<AircraftClass *>(O)) {
		Debug::Log("Casted to Aircraft - wtf?\n");
	}
	Debug::Log("Finished g_c test\n");
}


DEFINE_HOOK(48CCC0, Main_Game, 8)
{
	if(Ares::bTestingRun) {
//		TestScaffold::FloatToIntTest();
		return 0x48CFCB;
	}
	return 0;
}

#include "TestBox.h"

void TestScaffold::FloatToIntTest() {
	Debug::Log("Starting FloatToInt test\n");

	for(int i = 0; i < 10; ++i) {
		for(double f = -5.75; f <= 5.75; f += 0.05) {
			Debug::Log("%lf -> %I64d\n", f, Game::F2I(f));
		}
	}

	Debug::Log("Finished FloatToInt test\n");
}


DEFINE_HOOK(48CCC0, Main_Game, 8)
{
	if(Ares::bTestingRun) {
		TestScaffold::FloatToIntTest();
		return 0x48CFCB;
	}
	return 0;
}

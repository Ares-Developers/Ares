#ifndef ARES_SAVEGAMEDEF_H
#define ARES_SAVEGAMEDEF_H

// include this file whenever something is to be saved.

#include "Savegame.h"

namespace Savegame {
	template <typename T>
	bool ReadAresStream(AresStreamReader &Stm, T &Value, bool RegisterForChange) {
		// not implemented
		return true;
	}

	template <typename T>
	bool WriteAresStream(AresStreamWriter &Stm, const T &Value) {
		// not implemented
		return true;
	}
}

#endif

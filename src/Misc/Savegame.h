#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

#include "Stream.h"

namespace Savegame {
	template <typename T>
	bool ReadAresStream(AresStreamReader &Stm, T &Value, bool RegisterForChange = true) {
		// not implemented
		return false;
	};

	template <typename T>
	bool WriteAresStream(AresStreamWriter &Stm, const T &Value) {
		// not implemented
		return false;
	};
}

#endif

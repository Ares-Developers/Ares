#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

#include "Stream.h"

namespace Savegame {
	template <typename T>
	bool ReadAresStream(AresByteStream &Stm, T &Value, bool RegisterForChange = false) {
		// not implemented
		return false;
	};

	template <typename T>
	bool WriteAresStream(AresByteStream &Stm, const T &Value) {
		// not implemented
		return false;
	};
}

#endif

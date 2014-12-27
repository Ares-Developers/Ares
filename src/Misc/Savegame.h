#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

#include "Stream.h"

#include <type_traits>

namespace Savegame {
	template <typename T>
	bool ReadAresStream(AresStreamReader &Stm, T &Value, bool RegisterForChange = true);

	template <typename T>
	bool WriteAresStream(AresStreamWriter &Stm, const T &Value);

	template <typename T, typename = void>
	struct AresStreamObject {
		bool ReadFromStream(AresStreamReader &Stm, T &Value, bool RegisterForChange) const;

		bool WriteToStream(AresStreamWriter &Stm, const T &Value) const;
	};

	template <typename T>
	struct has_loadsave_members : public std::false_type {
	};
}

#define ENABLE_ARES_PERSISTENCE(type) \
template <> \
struct Savegame::has_loadsave_members<type> : public std::true_type { \
};

#endif

#pragma once

#include "Stream.h"

#include <memory>
#include <type_traits>

namespace Savegame {
	template <typename T>
	bool ReadAresStream(AresStreamReader &Stm, T &Value, bool RegisterForChange = true);

	template <typename T>
	bool WriteAresStream(AresStreamWriter &Stm, const T &Value);

	template <typename T>
	T* RestoreObject(AresStreamReader &Stm, bool RegisterForChange = true);

	template <typename T>
	bool PersistObject(AresStreamWriter &Stm, const T* pValue);

	template <typename T>
	struct AresStreamObject {
		bool ReadFromStream(AresStreamReader &Stm, T &Value, bool RegisterForChange) const;

		bool WriteToStream(AresStreamWriter &Stm, const T &Value) const;
	};

	template <typename T>
	struct ObjectFactory {
		std::unique_ptr<T> operator() (AresStreamReader &Stm) const {
			return std::make_unique<T>();
		}
	};
}

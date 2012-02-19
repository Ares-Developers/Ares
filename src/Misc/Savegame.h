#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

#include <windows.h>
#include <vector>

class AresByteStream : public std::vector<byte> {
protected:
	size_t CurrentOffset;
public:
	AresByteStream(size_t Reserve = 0x1000) : std::vector<byte>(), CurrentOffset(0) {
		this->reserve(Reserve);
	};

	bool ReadFromStream(IStream *pStm, const size_t Length) {
		this->reserve(this->size() + Length);
		auto tmp = new byte[Length];
		ULONG out;
		auto success = pStm->Read(tmp, Length, &out);
		bool result(SUCCEEDED(success) && out == Length);
		if(result) {
			this->insert(this->end(), tmp, tmp + Length);
		}
		delete[] tmp;
		return result;
	}

	template<typename T>
	bool Read(T &Value, const size_t Size) {
		if(this->size() <= this->CurrentOffset + Size) {
			return false;
		}
		auto Position = &this[this->CurrentOffset];
		this->CurrentOffset += Size;
		Value = *(reinterpret_cast<T *>(Position));
		return true;
	}
};

template<typename T>
class AresStreamWriter {
public:
	AresStreamWriter(AresByteStream &Stm, const T &Value);
};

template<typename T>
class AresStreamReader {
public:
	AresStreamReader(AresByteStream &Stm, T &Value, bool RegisterForChange = false);
};

template<typename T>
AresStreamWriter<T>::AresStreamWriter(AresByteStream &Stm, const T &Value) {
	const auto sz = sizeof(T);
	Stm.reserve(Stm.size() + sz);
	auto ptr = reinterpret_cast<const byte *>(&Value);
	Stm.insert(Stm.end(), ptr, ptr + sz);
};

template<typename T>
AresStreamReader<T>::AresStreamReader(AresByteStream &Stm, T &Value, bool RegisterForChange) {
	const auto sz = sizeof(T);
	Stm.Read(Value, sz);
	if(RegisterForChange) {
		AresSwizzle::Instance.RegisterPointerForChange(Value);
	}
	return true;
}

template<typename T>
inline bool AresByteStream::LoadFromStream(T &Value, const size_t Size, size_t &Offset, bool RegisterForChange) {
	const auto sz = sizeof(T);
	if(Offset >= Size) {
		return false;
	}
	if(Offset + sz > Size) {
		return false;
	}
	Offset += sz;
	return this->LoadFromStream(Value, RegisterForChange);
}

template<typename T>
inline bool AresByteStream::LoadValueFromStream(T &Value, const size_t Size) {
	return this->Read(Value, Size);
}

/* template specifications to save/load more complex types */
template<typename T>
inline bool AresByteStream::SaveToStream(const DynamicVectorClass<T> &Value) {
	this->SaveToStream(Value.Capacity);
	this->SaveToStream(Value.IsInitialized);
	this->SaveToStream(Value.Count);
	this->SaveToStream(Value.CapacityIncrement);

	for(auto ix = 0; ix < Value.Count; ++ix) {
		this->SaveToStream(Value.Items[ix]);
	}

	return true;
};

template<typename T>
inline bool AresByteStream::LoadValueFromStream(DynamicVectorClass<T> &Value, const size_t Size) {
	Value.Purge();
	int Capacity;
	this->LoadValueFromStream(Capacity, sizeof(Capacity));
	Value.SetCapacity(Capacity);
	int Count;
	this->LoadValueFromStream(Count, sizeof(Count));

	this->LoadValueFromStream(Value.CapacityIncrement, sizeof(Value.CapacityIncrement));
	for(auto ix = 0; ix < Count; ++ix) {
		this->LoadFromStream(Value[ix]);
	}
	return true;
};

#include "../Ext/BuildingType/PrismForwarding.h"
#include "../Ext/Building/PrismForwarding.h"

template<>
inline bool AresByteStream::SaveToStream(const BuildingExtras::cPrismForwarding &Value) {
	this->SaveToStream(Value.Senders);
	this->SaveToStream(Value.SupportTarget);
	this->SaveToStream(Value.PrismChargeDelay);
	this->SaveToStream(Value.DamageReserve);
	this->SaveToStream(Value.ModifierReserve);
	return true;
};

#endif

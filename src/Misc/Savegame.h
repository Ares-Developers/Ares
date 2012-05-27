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

	// IStream operations

	/**
	 * reads {Length} bytes from {pStm} into its storage
	 */
public:
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

	/**
	 * writes all internal storage to {pStm}
	 */
public:
	bool WriteToStream(IStream *pStm) {
		ULONG out;
		const size_t Length(this->size());
		auto pv = reinterpret_cast<void *>(this->data());
		auto success = pStm->Write(pv, Length, &out);
		return SUCCEEDED(success) && out == Length;
	}


	// primitive save/load - should not be specialized

	/**
	 * if it has {Size} bytes left, casts the first {Size} unread bytes to a {<T>} and assigns it to {Value}
	 * moves the internal position forward
	 */
public:
	template<typename T>
	bool Read(T &Value, const size_t Size = sizeof(T));

	/**
	 * ensures there are at least {Size} bytes left in the internal storage, casts those bytes to a {<T>} and assigns {Value} to that casted buffer
	 * moves the internal position forward
	 */
public:
	template<typename T>
	bool Write(const T &Value, const size_t Size);


	/**
	 * attempts to read the data from internal storage into {Value}
	 * updates {Offset} with the amount of data read, if successful
	 */
public:
	template<typename T>
	bool Load(T &Value, size_t &Offset);

	/**
	 * attempts to write the data from internal storage into {Value}
	 */
public:
	template<typename T>
	bool Save(const T &Value);
};

template<typename T>
bool AresByteStream::Read(T &Value, const size_t Size) {
	if(this->size() <= this->CurrentOffset + Size) {
		return false;
	}
	auto Position = &this[this->CurrentOffset];
	this->CurrentOffset += Size;
	Value = *(reinterpret_cast<T *>(Position));
	return true;
};

template<typename T>
bool AresByteStream::Write(const T &Value, const size_t Size) {
	if(this->size() <= this->CurrentOffset + Size) {
		this->reserve(this->CurrentOffset + Size);
	}
	auto Position = &this[this->CurrentOffset];
	this->CurrentOffset += Size;
	*(reinterpret_cast<T *>(Position)) = Value;
	return true;
};

template<typename T>
bool AresByteStream::Load(T &Value, size_t &Offset) {
	const auto sz = sizeof(T);
	if(Offset + sz > this->size()) {
		return false;
	}
	Offset += sz;
	return this->Read(Value, sz);
}

template<typename T>
bool AresByteStream::Save(const T &Value) {
	return this->Write(Value, sizeof(T));
};


// helper classes - voodoo power

template<typename T>
class AresStreamWriter {
public:
	AresStreamWriter(AresByteStream &Stm, const T &Value);
};

template<typename T>
class AresStreamReader {
public:
	AresStreamReader(AresByteStream &Stm, T &Value, size_t &Offset, bool RegisterForChange = false);
};


// helper functions for type inference
// what an onion
namespace Savegame {
	template <typename T>
	AresStreamReader<T> ReadAresStream(AresByteStream &Stm, T &Value, size_t &Offset, bool RegisterForChange = false)
		{ return AresStreamReader<T>(Stm, Value, Offset, RegisterForChange); }

	template <typename T>
	AresStreamReader<VectorClass<T>> ReadAresStream(AresByteStream &Stm, VectorClass<T> &Value, size_t &Offset, bool RegisterForChange)
		{ return AresStreamReader<VectorClass<T>>(Stm, Value, Offset, RegisterForChange); }

	template <typename T>
	AresStreamReader<DynamicVectorClass<T>> ReadAresStream(AresByteStream &Stm, DynamicVectorClass<T> &Value, size_t &Offset, bool RegisterForChange)
		{ return AresStreamReader<DynamicVectorClass<T>>(Stm, Value, Offset, RegisterForChange); }

	template <typename T>
	AresStreamWriter<T> WriteAresStream(AresByteStream &Stm, const T &Value)
		{ return AresStreamWriter<T>(Stm, Value); }

	template <typename T>
	AresStreamWriter<VectorClass<T>> WriteAresStream(AresByteStream &Stm, const VectorClass<T> &Value)
		{ return AresStreamWriter<VectorClass<T>>(Stm, Value); }

	template <typename T>
	AresStreamWriter<DynamicVectorClass<T>> WriteAresStream(AresByteStream &Stm, const DynamicVectorClass<T> &Value)
		{ return AresStreamWriter<DynamicVectorClass<T>>(Stm, Value); }
};


template<typename T>
AresStreamWriter<T>::AresStreamWriter(AresByteStream &Stm, const T &Value) {
	Stm.Save(Value);
};

template<typename T>
AresStreamReader<T>::AresStreamReader(AresByteStream &Stm, T &Value, size_t &Offset, bool RegisterForChange) {
	Stm.Load(Value, Offset);
	if(RegisterForChange) {
		AresSwizzle::Instance.RegisterPointerForChange(Value);
	}
}

// specializations

template<typename T>
class AresStreamWriter<DynamicVectorClass<T>> {
public:
	AresStreamWriter(AresByteStream &Stm, const DynamicVectorClass<T> &Value) {
		Stm.Save(Value.Capacity);
		Stm.Save(Value.IsInitialized);
		Stm.Save(Value.Count);
		Stm.Save(Value.CapacityIncrement);

		for(auto ix = 0; ix < Value.Count; ++ix) {
			Stm.Save(Value.Items[ix]);
		}
	};
};

template<typename T>
class AresStreamReader<DynamicVectorClass<T>> {
public:
	AresStreamReader(AresByteStream &Stm, DynamicVectorClass<T> &Value, size_t &Offset, bool RegisterForChange) {
		Value.Purge();
		int Capacity;
		Stm.Load(Capacity, Offset);
		Value.SetCapacity(Capacity);
		int Count;
		Stm.Load(Count, Offset);

		Stm.Load(Value.CapacityIncrement, sizeof(Value.CapacityIncrement));
		for(auto ix = 0; ix < Count; ++ix) {
			Stm.Load(Value[ix], Offset);
		}
		if(RegisterForChange) {
			for(auto ix = 0; ix < Count; ++ix) {
				AresSwizzle::Instance.RegisterPointerForChange(Value[ix]);
			}
		}
	}
};

#include "../Ext/BuildingType/PrismForwarding.h"
#include "../Ext/Building/PrismForwarding.h"

template<>
class AresStreamWriter<BuildingExtras::cPrismForwarding> {
public:
	AresStreamWriter(AresByteStream &Stm, const BuildingExtras::cPrismForwarding &Value) {
		Savegame::WriteAresStream(Stm, Value.Senders);
		Stm.Save(Value.SupportTarget);
		Stm.Save(Value.PrismChargeDelay);
		Stm.Save(Value.DamageReserve);
		Stm.Save(Value.ModifierReserve);
	}
};

#endif


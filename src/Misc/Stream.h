#ifndef ARES_STREAM_H
#define ARES_STREAM_H

#include <Objidl.h>
#include <vector>

class AresByteStream {
public:
	typedef unsigned char data_t;
protected:
	std::vector<data_t> Data;
	size_t CurrentOffset;
public:
	AresByteStream(size_t Reserve = 0x1000) : Data(), CurrentOffset(0) {
		this->Data.reserve(Reserve);
	};

	size_t Size() const {
		return this->Data.size();
	}

	size_t Offset() const {
		return this->CurrentOffset;
	}

	/**
	* reads {Length} bytes from {pStm} into its storage
	*/
	bool ReadFromStream(IStream *pStm, const size_t Length);

	/**
	* writes all internal storage to {pStm}
	*/
	bool WriteToStream(IStream *pStm) const;

	/**
	* reads the next block of bytes from {pStm} into its storage,
	* the block size is prepended to the block
	*/
	size_t ReadBlockFromStream(IStream *pStm);

	/**
	* writes all internal storage to {pStm}, prefixed with its length
	*/
	bool WriteBlockToStream(IStream *pStm) const;


	// primitive save/load - should not be specialized

	/**
	* if it has {Size} bytes left, assigns the first {Size} unread bytes to {Value}
	* moves the internal position forward
	*/
	bool Read(data_t* Value, size_t Size) {
		bool ret = false;
		if(this->Data.size() >= this->CurrentOffset + Size) {
			auto Position = &this->Data[this->CurrentOffset];
			std::memcpy(Value, Position, Size);
			ret = true;
		}

		this->CurrentOffset += Size;
		return ret;
	}

	/**
	* ensures there are at least {Size} bytes left in the internal storage, and assigns {Value} casted to byte to that buffer
	* moves the internal position forward
	*/
	void Write(const data_t* Value, size_t Size) {
		this->Data.insert(this->Data.end(), Value, Value + Size);
	}


	/**
	* attempts to read the data from internal storage into {Value}
	*/
	template<typename T>
	bool Load(T &Value) {
		// get address regardless of overloaded & operator
		auto Bytes = &reinterpret_cast<data_t&>(Value);
		return this->Read(Bytes, sizeof(T));
	}

	/**
	* writes the data from {Value} into internal storage
	*/
	template<typename T>
	void Save(const T &Value) {
		// get address regardless of overloaded & operator
		auto Bytes = &reinterpret_cast<const data_t&>(Value);
		this->Write(Bytes, sizeof(T));
	};
};

#endif

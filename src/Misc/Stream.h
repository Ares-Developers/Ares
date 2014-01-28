#ifndef ARES_STREAM_H
#define ARES_STREAM_H

#include <Objidl.h>
#include <vector>

class AresByteStream {
protected:
	typedef unsigned char data_t;
	std::vector<data_t> Data;
	size_t CurrentOffset;
public:
	AresByteStream(size_t Reserve = 0x1000) : Data(), CurrentOffset(0) {
		this->Data.reserve(Reserve);
	};

	size_t Size() const {
		return this->Data.size();
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
	bool AresByteStream::Read(data_t* Value, size_t Size) {
		if(this->Data.size() < this->CurrentOffset + Size) {
			return false;
		}
		auto Position = &this->Data[this->CurrentOffset];
		this->CurrentOffset += Size;

		std::memcpy(Value, Position, Size);
		return true;
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
	* updates {Offset} with the amount of data read, no matter if successful
	* this is done to ensure once an item could not be read, the subsequent items will fail also
	*/
	template<typename T>
	bool Load(T &Value, size_t &Offset) {
		Offset += sizeof(T);

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

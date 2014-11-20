#ifndef ARES_STREAM_H
#define ARES_STREAM_H

#include <vector>

struct IStream;

class AresByteStream {
public:
	typedef unsigned char data_t;
protected:
	std::vector<data_t> Data;
	size_t CurrentOffset;
public:
	AresByteStream(size_t Reserve = 0x1000);

	~AresByteStream();

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

class AresStreamWorkerBase {
public:
	explicit AresStreamWorkerBase(AresByteStream& Stream) : stream(&Stream), success(true) {}
	AresStreamWorkerBase(const AresStreamWorkerBase&) = delete;

	AresStreamWorkerBase& operator = (const AresStreamWorkerBase&) = delete;

	bool Success() const {
		return this->success;
	}

protected:
	AresByteStream* stream;
	bool success;
};

class AresStreamReader : public AresStreamWorkerBase {
public:
	explicit AresStreamReader(AresByteStream& Stream) : AresStreamWorkerBase(Stream) {}
	AresStreamReader(const AresStreamReader&) = delete;

	AresStreamReader& operator = (const AresStreamReader&) = delete;

	template <typename T>
	AresStreamReader& Process(T& value, bool RegisterForChange = true) {
		if(this->Success()) {
			this->success &= Savegame::ReadAresStream(*this, value, RegisterForChange);
		}
		return *this;
	}

	// helpers

	bool ExpectEndOfBlock() const {
		if(!this->Success() || this->stream->Size() != this->stream->Offset()) {
			this->EmitExpectEndOfBlockWarning();
			return false;
		}
		return true;
	}

	template <typename T>
	bool Load(T& buffer) {
		if(!this->stream->Load(buffer)) {
			this->EmitLoadWarning(sizeof(T));
			this->success = false;
			return false;
		}
		return true;
	}

	bool Read(AresByteStream::data_t* Value, size_t Size) {
		if(!this->stream->Read(Value, Size)) {
			this->EmitLoadWarning(Size);
			this->success = false;
			return false;
		}
		return true;
	}

	bool Expect(unsigned int value) {
		unsigned int buffer = 0;
		if(this->Load(buffer)) {
			if(buffer == value) {
				return true;
			}

			this->EmitExpectWarning(buffer, value);
		}
		return false;
	}

	bool RegisterChange(void* newPtr);

private:
	void EmitExpectEndOfBlockWarning() const;
	void EmitLoadWarning(size_t size) const;
	void EmitExpectWarning(unsigned int found, unsigned int expect) const;
	void EmitSwizzleWarning(long id, void* pointer) const;
};

class AresStreamWriter : public AresStreamWorkerBase {
public:
	explicit AresStreamWriter(AresByteStream& Stream) : AresStreamWorkerBase(Stream) {}
	AresStreamWriter(const AresStreamWriter&) = delete;

	AresStreamWriter& operator = (const AresStreamWriter&) = delete;

	template <typename T>
	AresStreamWriter& Process(T& value, bool RegisterForChange = true) {
		if(this->Success()) {
			this->success &= Savegame::WriteAresStream(*this, value);
		}
		return *this;
	}

	// helpers

	template <typename T>
	void Save(const T& buffer) {
		this->stream->Save(buffer);
	}

	void Write(const AresByteStream::data_t* Value, size_t Size) {
		this->stream->Write(Value, Size);
	}

	bool Expect(unsigned int value) {
		this->Save(value);
		return true;
	}

	bool RegisterChange(const void* oldPtr) {
		this->Save(oldPtr);
		return true;
	}
};

#endif

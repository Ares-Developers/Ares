#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

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
};

#endif

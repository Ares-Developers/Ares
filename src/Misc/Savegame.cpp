#include "Savegame.h"

bool AresByteStream::ReadFromStream(IStream *pStm, const size_t Length) {
	this->Data.reserve(this->Data.size() + Length);
	auto tmp = new data_t[Length];
	ULONG out = 0;
	auto success = pStm->Read(tmp, Length, &out);
	bool result(SUCCEEDED(success) && out == Length);
	if(result) {
		this->Data.insert(this->Data.end(), tmp, tmp + Length);
	}
	delete[] tmp;
	return result;
}

bool AresByteStream::WriteToStream(IStream *pStm) const {
	ULONG out = 0;
	const size_t Length(this->Data.size());
	auto pcv = reinterpret_cast<const void *>(this->Data.data());
	auto success = pStm->Write(pcv, Length, &out);
	return SUCCEEDED(success) && out == Length;
}

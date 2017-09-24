#include "Stream.h"

#include <Objidl.h>

bool AresByteStream::ReadFromStream(IStream *pStm, const size_t Length) {
	ULONG out = 0;
	auto size = this->Data.size();
	this->Data.resize(size + Length);
	auto pv = reinterpret_cast<void *>(this->Data.data());
	auto success = pStm->Read(pv, Length, &out);
	bool result(SUCCEEDED(success) && out == Length);
	if(!result) {
		this->Data.resize(size);
	}
	return result;
}

bool AresByteStream::WriteToStream(IStream *pStm) const {
	ULONG out = 0;
	const size_t Length(this->Data.size());
	auto pcv = reinterpret_cast<const void *>(this->Data.data());
	auto success = pStm->Write(pcv, Length, &out);
	return SUCCEEDED(success) && out == Length;
}

size_t AresByteStream::ReadBlockFromStream(IStream *pStm) {
	ULONG out = 0;
	size_t Length = 0;
	if(SUCCEEDED(pStm->Read(&Length, sizeof(Length), &out))) {
		if(this->ReadFromStream(pStm, Length)) {
			return Length;
		}
	}
	return 0;
}

bool AresByteStream::WriteBlockToStream(IStream *pStm) const {
	ULONG out = 0;
	const size_t Length = this->Data.size();
	if(SUCCEEDED(pStm->Write(&Length, sizeof(Length), &out))) {
		return this->WriteToStream(pStm);
	}
	return false;
}

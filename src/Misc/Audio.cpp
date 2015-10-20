// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include "../Ares.h"
#include "../Ares.CRT.h"

LooseAudioCache LooseAudioCache::Instance;
AudioLuggage AudioLuggage::Instance;

AudioSampleData* AresAudioHelper::GetData(int const index)
{
	if(auto pFilename = ToLooseFile(index)) {
		auto& loose = LooseAudioCache::Instance.GetData(pFilename);

		if(loose.Size < 0) {
			auto file = GetFile(index);
			if(file.File && file.Allocated) {
				GameDelete(file.File);
			}
		}

		return &loose.Data;
	}

	return nullptr;
}

AresAudioHelper::FileStruct AresAudioHelper::GetFile(int const index)
{
	if(auto const pSampleName = ToLooseFile(index)) {
		auto& loose = LooseAudioCache::Instance.GetData(pSampleName);

		// Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100];
		_snprintf_s(filename, _TRUNCATE, "%s.wav", pSampleName);

		auto pFile = GameCreate<CCFileClass>(filename);

		if(pFile->Exists() && pFile->Open(FileAccessMode::Read)) {
			if(loose.Size < 0 && Audio::ReadWAVFile(pFile, &loose.Data, &loose.Size)) {
				loose.Offset = pFile->Seek(0, FileSeekMode::Current);
			}
		} else {
			GameDelete(pFile);
			pFile = nullptr;
		}

		return FileStruct{ loose.Size, loose.Offset, pFile, true };
	} else {
		auto& sample = AudioIDXData::Instance->Samples[index];
		auto const pFile = AudioLuggage::Instance.GetFile(index);
		return FileStruct{ sample.Size, sample.Offset, pFile, false };
	}
}

void AudioBag::Open(const char* fileBase) {
	char filename[0x100];
	_snprintf_s(filename, _TRUNCATE, "%s.idx", fileBase);

	auto pIndex = UniqueGamePtr<CCFileClass>(GameCreate<CCFileClass>(filename));

	if(pIndex->Exists() && pIndex->Open(FileAccessMode::Read)) {
		_snprintf_s(filename, _TRUNCATE, "%s.bag", fileBase);
		auto pBag = UniqueGamePtr<CCFileClass>(GameCreate<CCFileClass>(filename));

		if(pBag->Exists() && pBag->Open(FileAccessMode::Read)) {
			AudioIDXHeader headerIndex;

			if(pIndex->Read(headerIndex)) {
				std::vector<AudioIDXEntry> entries;

				if(headerIndex.numSamples > 0) {
					entries.resize(headerIndex.numSamples, {});

					auto const Size = sizeof(AudioIDXEntry);

					if(headerIndex.Magic == 1) {
						for(auto& entry : entries) {
							if(!pIndex->Read(entry, Size - 4)) {
								return;
							}
							entry.ChunkSize = 0;
						}
					} else {
						auto const headerSize = headerIndex.numSamples * Size;
						if(!pIndex->Read(entries[0], static_cast<int>(headerSize))) {
							return;
						}
					}
				}

				std::sort(entries.begin(), entries.end());
				this->Bag = std::move(pBag);
				this->Entries = std::move(entries);
			}
		}
	}
}

AudioIDXData* AudioLuggage::Create(const char* pPath) {
	std::map<AudioIDXEntry, CCFileClass*> map;

	for(auto const& bag : this->Bags) {
		for(auto const& entry : bag.entries()) {
			map[entry] = bag.file();
		}
	}

	auto ret = GameCreate<AudioIDXData>();
	ret->BagFile = nullptr; // this->Bags.front().file(); // not needed
	ret->SampleCount = static_cast<int>(map.size());
	ret->Samples = GameCreateArray<AudioIDXEntry>(map.size());

	this->Files.clear();
	this->Files.reserve(map.size());

	auto pEntry = ret->Samples;

	for(auto const& item : map) {
		*pEntry++ = item.first;
		this->Files.push_back(item.second);
	}

#ifdef SUPPORT_PATH
	if(pPath) {
		auto const lenPath = strlen(pPath);

		strcpy_s(ret->Path, pPath);
		if(ret->Path[lenPath - 1] == '\\') {
			ret->Path[lenPath - 1] = '\0';
		}

		WIN32_FIND_DATA find;
		auto const hFind = FindFirstFileA(ret->Path, &find);

		if(hFind != INVALID_HANDLE_VALUE) {
			do {
				if(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					ret->PathFound = TRUE;
					break;
				}
			} while(FindNextFileA(hFind, &find));

			FindClose(hFind);
		}

		if(lenPath < std::size(ret->Path)) {
			ret->Path[lenPath - 1] = '\\';
			ret->Path[lenPath] = '\0';
		}
	}
#endif

	return ret;
}

// load more than one audio bag and index.
// this replaces the entire old parser.
DEFINE_HOOK(4011C0, Audio_Load, 6)
{
	auto& luggage = AudioLuggage::Instance;

	// audio.bag and ares.bag
	luggage.Append("audio");
	luggage.Append("ares");

	// audio01.bag to audio99.bag
	char buffer[0x100];
	for(auto i = 1; i < 100; ++i) {
		_snprintf_s(buffer, _TRUNCATE, "audio%02d", i);
		luggage.Append(buffer);
	}

	// generate index
	R->EAX(luggage.Create());
	return 0x401578;
}

DEFINE_HOOK(4064A0, VocClass_AddSample, 0) // Complete rewrite of VocClass::AddSample
{
	GET(VocClass*, pVoc, ECX);
	GET(const char*, pSampleName, EDX);

	if(pVoc->NumSamples == 0x20) {
		// return false
		R->EAX(0);
	} else {
		if(*reinterpret_cast<int*>(0x87E2A0)) { // I dunno
			while(*pSampleName == '$' || *pSampleName == '#') {
				++pSampleName;
			}

			auto idxSample = !AudioIDXData::Instance ? -1
				: AudioIDXData::Instance->FindSampleIndex(pSampleName);

			if(idxSample == -1) {
				idxSample = LooseAudioCache::Instance.GetIndex(pSampleName);
			}

			// Set sample index or string pointer
			pVoc->SampleIndex[pVoc->NumSamples] = idxSample;
			++pVoc->NumSamples;

			// return true
			R->EAX(1);
		}
	}

	return 0x40651E;
}

DEFINE_HOOK(4016F0, IDXContainer_LoadSample, 6)
{
	GET(AudioIDXData* const, pThis, ECX);
	GET(int const, index, EDX);

	pThis->ClearCurrentSample();

	auto const file = AresAudioHelper::GetFile(index);
	pThis->CurrentSampleFile = file.File;
	pThis->CurrentSampleSize = file.Size;
	if(file.Allocated) {
		pThis->ExternalFile = file.File;
	}

	auto const ret = file.File && file.Size
		&& file.File->Seek(file.Offset, FileSeekMode::Set) == file.Offset;

#ifdef SUPPORT_PATH
	// not expnded, does not handle loose files (like freeing current file)
	if(pThis->PathFound != FALSE) {
		auto& sample = pThis->Samples[index];

		char filename[MAX_PATH];
		_snprintf_s(filename, _TRUNCATE, "%s%s.wav", pThis->Path, sample.Name);

		// warning: this will leak loose files! -AlexB
		auto const pFile = GameCreate<RawFileClass>(filename);
		pThis->ExternalFile = pFile;

		if(pFile->Exists() && pFile->Open(FileAccessMode::Read)) {
			AudioSampleData data;
			int sampleSize;

			if(Audio::ReadWAVFile(pFile, &data, &sampleSize)) {
				sample.Flags = 2;
				if(data.BytesPerSample == 2) {
					sample.Flags = 6;
				}
				if(data.NumChannels == 2) {
					sample.Flags |= 1;
				}
				sample.SampleRate = data.SampleRate;
				pThis->CurrentSampleSize = sampleSize;
				pThis->CurrentSampleFile = pThis->ExternalFile;
				R->EAX(true);
				return 0x4018B8;
			}
		}

		if(pThis->ExternalFile) {
			GameDelete(pThis->ExternalFile);
			pThis->ExternalFile = nullptr;
		}
	}
#endif

	R->EAX(ret);
	return 0x4018B8;
}

DEFINE_HOOK(401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if(auto const pData = AresAudioHelper::GetData(idxSample)) {
		if(pData->SampleRate) {
			*pAudioSample = *pData;
		} else {
			pAudioSample->Data = 4;
			pAudioSample->Format = 0;
			pAudioSample->SampleRate = 22050;
			pAudioSample->NumChannels = 1;
			pAudioSample->BytesPerSample = 2;
			pAudioSample->BlockAlign = 0;
		}

		R->EAX(pAudioSample);
		return 0x40169E;
	}

	return 0;
}

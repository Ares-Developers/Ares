// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include "../Ares.h"
#include "../Ares.CRT.h"

LooseAudioCache LooseAudioCache::Instance;

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

		return FileStruct{ pFile, true };
	} else {
		return FileStruct{ AudioIDXData::Instance->BagFile, false };
	}
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

DEFINE_HOOK(4016F7, AudioIndex_LoadSample, 5) //50% rewrite of Audio::LoadWAV
{
	GET(const int, idxSample, EDX);

	if(idxSample >= MinimumAresSample) {
		auto const pSampleName = reinterpret_cast<const char*>(idxSample);

		GET(AudioIDXData*, pAudioIndex, ECX);
		pAudioIndex->ClearCurrentSample();

		// Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100];
		_snprintf_s(filename, _TRUNCATE, "%s.wav", pSampleName);

		auto const pFile = GameCreate<CCFileClass>(filename);
		pAudioIndex->ExternalFile = pFile;

		if(pFile->Exists()) {
			if(pFile->Open(FileAccessMode::Read)) {
				AudioSampleData Data;
				int nSampleSize;

				if(Audio::ReadWAVFile(pFile, &Data, &nSampleSize)) {
					pAudioIndex->CurrentSampleFile = pFile;
					pAudioIndex->CurrentSampleSize = nSampleSize;

					R->EAX(1);
					return 0x401889;
				}
			}
		}

		pAudioIndex->ExternalFile = nullptr;
		GameDelete(pFile);

		R->EAX(0);
		return 0x401889;
	}

	return 0;
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

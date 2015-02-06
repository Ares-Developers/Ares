// Allows WAV files being placed in Mixes

#include <Audio.h>
#include <CCFileClass.h>
#include <VocClass.h>
#include "../Ares.h"
#include "../Ares.CRT.h"

#include <set>
#include <string>

// do not change! this is not a game constant, but a technical one.
// memory will not be allocated below this address, thus only values
// below are guaranteed to be indexes. -AlexB
auto const MinimumAresSample = 0x10000;

class LooseAudioCache {
public:
	int GetIndex(const char* pFilename) {
		auto it = this->Files.find(pFilename);
		if(it == this->Files.end()) {
			it = this->Files.emplace(pFilename).first;
		}

		return reinterpret_cast<int>(it->c_str());
	}

private:
	std::set<std::string> Files;
};

LooseAudioCache LooseFiles;

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
				idxSample = LooseFiles.GetIndex(pSampleName);
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
		char filename[0x100] = "\0";
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

	if(idxSample >= MinimumAresSample) {
		pAudioSample->Data = 4;
		pAudioSample->Format = 0;
		pAudioSample->SampleRate = 22050;
		pAudioSample->NumChannels = 1;
		pAudioSample->BytesPerSample = 2;
		pAudioSample->BlockAlign = 0;

		R->EAX(pAudioSample);
		return 0x40169E;
	}

	return 0;
}

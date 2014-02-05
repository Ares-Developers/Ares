//Allows WAV files being placed in Mixes

#include <Audio.h>
#include <CCFileClass.h>
#include <VocClass.h>
#include "../Ares.h"
#include "../Ares.CRT.h"

// assuming nobody has 128k legit samples in their game
#define MINIMUM_ARES_SAMPLE 0x20000

DEFINE_HOOK(4064A0, Ares_Audio_AddSample, 0)	//Complete rewrite of VocClass::AddSample
{
	//TODO: Once a VocClass and AudioIDXData definition is available, rewrite this. -pd
	GET(VocClass*, pVoc, ECX); //VocClass*
	GET(char*, pSampleName, EDX);

	if(pVoc->NumSamples == 0x20) { //if(pVoc->get_NumSamples()==0x20)
		R->EAX(0); //return false
	} else {
		if(*((int*)0x87E2A0)) //I dunno
		{
			while(*pSampleName == '$' || *pSampleName == '#') {
				++pSampleName;
			}

			int nSampleIndex = (AudioIDXData::IDX)
				? AudioIDXData::IDX->FindSampleIndex(pSampleName)
				: -1
			;

			if(nSampleIndex == -1) {
				nSampleIndex = (int)_strdup(pSampleName);
			}

			pVoc->SampleIndex[pVoc->NumSamples] = nSampleIndex; //Set sample index or string pointer
			++pVoc->NumSamples;

			R->EAX(1); //return true
		}
	}
	return 0x40651E;
}

//Hook at 0x75144F AND 0x75048E (?? that address makes no sense)
DEFINE_HOOK(75144F, Ares_Audio_DeleteSampleNames, 9)
//FINE_HOOK_AGAIN(75048E, Ares_Audio_DeleteSampleNames, 9)
{
	//TODO: Once a VocClass definition is available, rewrite this. -pd

	GET(VocClass **, ppVoc, ESI);
	if(VocClass *pVoc = *ppVoc) {
		for(int i=0; i < pVoc->NumSamples; ++i) {
			int SampleIndex = pVoc->SampleIndex[i];	//SampleIndex[i]
			if(SampleIndex >= MINIMUM_ARES_SAMPLE) {
				free((char*)SampleIndex);
			}
		}
		delete ppVoc;
	}
	return R->get_Origin() + 9;
}


DEFINE_HOOK(75048E, VocClass_LoadFromINI_ResetSamples, 9)
{
	GET(VocClass **, ppVoc, ESI);
	if(VocClass *pVoc = *ppVoc) {
		for(int i=0; i < pVoc->NumSamples; ++i) {
			int SampleIndex = pVoc->SampleIndex[i];	//SampleIndex[i]
			if(SampleIndex >= MINIMUM_ARES_SAMPLE) {
				free((char*)SampleIndex);
			}
		}
		pVoc->NumSamples = 0;
	}
	return 0;
}


DEFINE_HOOK(4016F7, Ares_Audio_LoadWAV, 5)	//50% rewrite of Audio::LoadWAV
{
	//TODO: Once an AudioIndex definition is available, rewrite this. -pd
	GET(int, SampleIndex, EDX);

	if(SampleIndex >= MINIMUM_ARES_SAMPLE) {
		char* SampleName=(char*)SampleIndex;

		GET(DWORD *, pAudioIndex, ECX);	//AudioIndex*
		pAudioIndex[0x110 >> 2] = 0;	//ExternalFile = nullptr
		pAudioIndex[0x118 >> 2] = 0;	//CurrentSampleFile = nullptr

		//Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100] = "\0";
		AresCRT::strCopy(filename, SampleName, 0x100 - 5);
		strcat(filename, ".wav");
		filename[0xFF] = 0;

		CCFileClass* pFile = GameCreate<CCFileClass>(filename);
		pAudioIndex[0x110 >> 2] = (DWORD)pFile;	//ExternalFile = pFile

		if(pFile->Exists(nullptr)) {
			if(pFile->Open(eFileMode::Read)) {
				int WAVStruct[0x8];
				int nSampleSize;

				if(Audio::ReadWAVFile(pFile, (void *)WAVStruct, &nSampleSize)) {
					pAudioIndex[0x118 >> 2] = (DWORD)pFile;	//CurrentSampleFile = pFile
					pAudioIndex[0x11C >> 2] = nSampleSize;	//CurrentSampleSize = nSampleSize
					R->EAX(1);
					return 0x401889;
				}
			}
		}
		pAudioIndex[0x110 >> 2] = 0;	//ExternalFile = nullptr
		GameDelete(pFile);

		R->EAX(0);
		return 0x401889;
	}

	return 0;
}

DEFINE_HOOK(401640, Ares_Audio_GetSampleInfo, 5)
{
	//TODO: Once an AudioSample definition is available (if ever), rewrite this. -pd
	GET(int, SampleIndex, EDX);
	if(SampleIndex >= MINIMUM_ARES_SAMPLE) {

		GET_STACK(int *, pAudioSample, 0x4);	//AudioSample*

		pAudioSample[0x00 >> 2] = 4;
		pAudioSample[0x04 >> 2] = 0;
		pAudioSample[0x08 >> 2] = 22050;
		pAudioSample[0x0C >> 2] = 1;
		pAudioSample[0x10 >> 2] = 2;
		pAudioSample[0x18 >> 2] = 0;

		R->EAX(pAudioSample);
		return 0x40169E;
	}
	return 0;
}


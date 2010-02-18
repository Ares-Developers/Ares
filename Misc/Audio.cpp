//Allows WAV files being placed in Mixes

#include <CCFileClass.h>
#include "../Ares.h"

DEFINE_HOOK(4064A0, Ares_Audio_AddSample, 0)	//Complete rewrite of VocClass::AddSample
{
	//TODO: Once a VocClass and AudioIDXData definition is available, rewrite this. -pd
	GET(int*, pVoc, ECX); //VocClass*
	GET(char*, pSampleName, EDX);

	if(pVoc[0x134 >> 2]==0x20) { //if(pVoc->get_NumSamples()==0x20)
		R->EAX(0); //return false
	} else {
		if(*((int*)0x87E2A0)) //I dunno
		{
			while(*pSampleName == '$' || *pSampleName == '#') {
				++pSampleName;
			}

			int nSampleIndex = -1;
			void* pAudioIDXData = *((void**)0x87E294);
			if(pAudioIDXData) {
				SET_REG32(edx, pSampleName);
				SET_REG32(ecx, pAudioIDXData);
				CALL(0x4015C0); //nSampleIndex = FindSampleIndex(pSampleName)
				GET_REG32(nSampleIndex, eax);
			}

			if(nSampleIndex == -1) {
				nSampleIndex = (int)_strdup(pSampleName);
			}

			pVoc[(0xB4 >> 2) + pVoc[0x134 >> 2]] = nSampleIndex; //Set sample index or string pointer
			++pVoc[0x134 >> 2];

			R->EAX(1); //return true
		}
	}
	return 0x40651E;
}

//Hook at 0x75144F AND 0x75048E
DEFINE_HOOK(75144F, Ares_Audio_DeleteSampleNames, 9)
DEFINE_HOOK_AGAIN(75048E, Ares_Audio_DeleteSampleNames, 9)
{
	//TODO: Once a VocClass definition is available, rewrite this. -pd

	GET(int **, ppVoc, ESI);
	int* pVoc = *ppVoc; //VocClass*
	if(pVoc) {
		//pVoc[0x134>>2] = NumSamples
		for(int i=0; i < pVoc[0x134 >> 2]; i++) {
			int SampleIndex = pVoc[i + (0xB4 >> 2)];	//SampleIndex[i]
			if(SampleIndex >= 0x400000) {
				//if greater than 0x400000, it's the SampleName allocated by us
				delete (char*)SampleIndex;
			}
		}
	}
	return 0;
}

DEFINE_HOOK(4016F7, Ares_Audio_LoadWAV, 5)	//50% rewrite of Audio::LoadWAV
{
	//TODO: Once an AudioIndex definition is available, rewrite this. -pd
	GET(int, SampleIndex, EDX);

	//if greater than 0x400000, it's the SampleName allocated by us
	if(SampleIndex>=0x400000) {
		char* SampleName=(char*)SampleIndex;

		GET(DWORD *, pAudioIndex, EBX);	//AudioIndex*
		pAudioIndex[0x110 >> 2] = 0;	//ExternalFile = NULL
		pAudioIndex[0x118 >> 2] = 0;	//CurrentSampleFile = NULL

		//Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100] = "\0";
		strncpy(filename, SampleName, 0x100);
		strcat(filename, ".wav");

		CCFileClass* pFile;
		GAME_ALLOC(CCFileClass, pFile, filename);
		pAudioIndex[0x110 >> 2] = (DWORD)pFile;	//ExternalFile = pFile

		if(pFile->Exists(NULL)) {
			if(pFile->Open(eFileMode::Read)) {
				int WAVStruct[0x8];
				int nSampleSize;
				int nResult;

				int* pSampleSize = &nSampleSize;

				PUSH_VAR32(pSampleSize);
				SET_REG32(edx,WAVStruct);
				SET_REG32(ecx,pFile);
				CALL(0x408610);
				GET_REG32(nResult,eax);

				if(nResult) {
					pAudioIndex[0x118 >> 2] = (DWORD)pFile;	//CurrentSampleFile = pFile
					pAudioIndex[0x11C >> 2] = nSampleSize;	//CurrentSampleSize = nSampleSize
					R->EAX(1);
					return 0x401889;
				}
			}
		}

		GAME_DEALLOC(pFile);
		pAudioIndex[0x110 >> 2] = NULL;	//ExternalFile = NULL

		R->EAX(0);
		return 0x401889;
	}

	return 0;
}

DEFINE_HOOK(401642, Ares_Audio_GetSampleInfo, 6)
{
	//TODO: Once an AudioSample definition is available (if ever), rewrite this. -pd
	GET(int, SampleIndex, EDX);
	//if greater than 0x400000, it's the SampleName allocated by us
	if(SampleIndex >= 0x400000) {
		// gcc: unused char* SampleName=(char*)SampleIndex;

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


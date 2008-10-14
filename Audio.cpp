//Allows WAV files being placed in Mixes

#include <CCFileClass.h>

//Hook at 0x4064A0
EXPORT Ares_Audio_AddSample(REGISTERS* R)	//Complete rewrite of VocClass::AddSample
{
	//TODO: Once a VocClass and AudioIDXData definition is available, rewrite this. -pd
	int* pVoc=(int*)R->get_ECX(); //VocClass*
	char* pSampleName=(char*)R->get_EDX();

	if(pVoc[0x134 >> 2]==0x20) //if(pVoc->get_NumSamples()==0x20)
		R->set_EAX(0); //return false
	else
	{
		if(*((int*)0x87E2A0)) //I dunno
		{
			while(*pSampleName == '$' || *pSampleName == '#')
				++pSampleName;

			int nSampleIndex = -1;
			void* pAudioIDXData = *((void**)0x87E294);
			if(pAudioIDXData)
			{
				SET_REG32(edx, pSampleName);
				SET_REG32(ecx, pAudioIDXData);
				CALL(0x4015C0); //nSampleIndex = FindSampleIndex(pSampleName)
				GET_REG32(nSampleIndex, eax);
			}

			if(nSampleIndex == -1)
				nSampleIndex = (int)_strdup(pSampleName);

			pVoc[(0xB4 >> 2) + pVoc[0x134 >> 2]] = nSampleIndex; //Set sample index or string pointer
			++pVoc[0x134 >> 2];

			R->set_EAX(1); //return true
		}
	}
	return 0x40651E;
}

//Hook at 0x75144F AND 0x75048E
EXPORT Ares_Audio_DeleteSampleNames(REGISTERS* R)
{
	//TODO: Once a VocClass definition is available, rewrite this. -pd

	int* pVoc = *((int**)R->get_ESI());	//VocClass*
	if(pVoc)
	{
		for(int i=0; i < pVoc[0x134 >> 2]; i++)	//pVoc[0x134>>2] = NumSamples
		{
			int SampleIndex = pVoc[i + (0xB4 >> 2)];	//SampleIndex[i]
			if(SampleIndex >= 0x400000)	//if greater than 0x400000, it's the SampleName allocated by us
				delete (char*)SampleIndex;
		}
	}
	return 0;
}

//Hook at 0x4016F9
EXPORT Ares_Audio_LoadWAV(REGISTERS* R)	//50% rewrite of Audio::LoadWAV
{
	//TODO: Once an AudioIndex definition is available, rewrite this. -pd
	int SampleIndex = R->get_EDX();

	if(SampleIndex>=0x400000)	//if greater than 0x400000, it's the SampleName allocated by us
	{
		char* SampleName=(char*)SampleIndex;

		DWORD* pAudioIndex=(DWORD*)R->get_EBX();	//AudioIndex*
		pAudioIndex[0x110 >> 2] = 0;	//ExternalFile = NULL
		pAudioIndex[0x118 >> 2] = 0;	//CurrentSampleFile = NULL

		//Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100] = "\0";
		strncpy(filename, SampleName, 0x100);
		strcat(filename, ".wav");

		CCFileClass* pFile = new CCFileClass(filename);
		pAudioIndex[0x110 >> 2] = (DWORD)pFile;	//ExternalFile = pFile

		if(pFile->Exists(NULL))
		{
			if(pFile->Open(FILE_READ_ACCESS))
			{
				int WAVStruct[0x8];
				int nSampleSize;
				int nResult;

				int* pSampleSize=&nSampleSize;

				PUSH_VAR32(pSampleSize);
				SET_REG32(edx,WAVStruct);
				SET_REG32(ecx,pFile);
				CALL(0x408610);
				GET_REG32(nResult,eax);

				if(nResult)
				{
					pAudioIndex[0x118 >> 2] = (DWORD)pFile;	//CurrentSampleFile = pFile
					pAudioIndex[0x11C >> 2] = nSampleSize;	//CurrentSampleSize = nSampleSize
					R->set_EAX(1);
					return 0x401889;
				}
			}
		}

		delete pFile;
		pAudioIndex[0x110 >> 2] = NULL;	//ExternalFile = NULL

		R->set_EAX(0);
		return 0x401889;
	}

	return 0;
}

//Hook at 0x401642
EXPORT Ares_Audio_GetSampleInfo(REGISTERS* R)
{
	//TODO: Once an AudioSample definition is available (if ever), rewrite this. -pd
	int SampleIndex = R->get_EDX();
	if(SampleIndex >= 0x400000)	//if greater than 0x400000, it's the SampleName allocated by us
	{
		// gcc: unused char* SampleName=(char*)SampleIndex;

		int* pAudioSample = (int*)R->get_StackVar32(0x4);	//AudioSample*
		
		pAudioSample[0x00 >> 2] = 4;
		pAudioSample[0x04 >> 2] = 0;
		pAudioSample[0x08 >> 2] = 22050;
		pAudioSample[0x0C >> 2] = 1;
		pAudioSample[0x10 >> 2] = 2;
		pAudioSample[0x18 >> 2] = 0;

		R->set_EAX((DWORD)pAudioSample);
		return 0x40169E;
	}
	return 0;
}

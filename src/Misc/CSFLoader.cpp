#include "CSFLoader.h"
#include <cstdio>

#include "../Ares.h"

int CSFLoader::CSFCount = 0;
int CSFLoader::NextValueIndex = 0;
std::hash_map<std::string, const CSFString*> CSFLoader::DynamicStrings;

void CSFLoader::LoadAdditionalCSF(const char *pFileName)
{
	//The main stringtable must have been loaded (memory allocation)
	//To do that, use StringTable::LoadFile.
	if(StringTable::IsLoaded && pFileName && *pFileName) {
		CCFileClass* pFile;
		GAME_ALLOC(CCFileClass, pFile, pFileName);
		if(pFile->Exists(NULL) && pFile->Open(eFileMode::Read)) {
			CSFHeader header;

			if(pFile->ReadBytes(&header, sizeof(CSFHeader)) == sizeof(CSFHeader)) {
				if(header.Signature == CSF_SIGNATURE &&
					header.CSFVersion >= 2 &&
					header.Language == StringTable::Language) //should stay in one language
				{
					++CSFCount;
					StringTable::ReadFile(pFileName); //must be modified to do the rest ;)

					qsort(
						StringTable::Labels,
						StringTable::LabelCount,
						sizeof(CSFLabel),
						(int (__cdecl *)(const void *,const void *))_strcmpi);
				}
			}
		}
		GAME_DEALLOC(pFile);
	}
};

const CSFString* CSFLoader::FindDynamic(const char* pLabelName) {
	if(pLabelName) {
		auto it	= DynamicStrings.find(pLabelName);
		if(it != DynamicStrings.end()) {
			return it->second;
		}
	}

	// failed
	return NULL;
}

const wchar_t* CSFLoader::GetDynamicString(const char* pLabelName, const wchar_t* pPattern, const char* pDefault) {
	const CSFString *String = CSFLoader::FindDynamic(pLabelName);

	if(!String) {
		CSFString* NewString = NULL;
		GAME_ALLOC(CSFString, NewString);
		wsprintfW(NewString->Text, pPattern, pDefault);

		NewString->PreviousEntry = StringTable::LastLoadedString;
		StringTable::LastLoadedString = NewString;

		if(Ares::bOutputMissingStrings) {
			Debug::Log("[CSFLoader] Added label \"%s\" with value \"%ls\".\n", pLabelName, NewString->Text);
		}

		DynamicStrings[pLabelName] = NewString;
		String = NewString;
	}

	return String->Text;
}

//0x7346D0
DEFINE_HOOK(7346D0, CSF_LoadBaseFile, 6)
{
	StringTable::IsLoaded = true;
	CSFLoader::CSFCount = 0;

	return 0;
}

//0x734823
DEFINE_HOOK(734823, CSF_AllocateMemory, 6)
{
	//aaaah... finally, some serious hax :)
	//we don't allocate memory by the amount of labels in the base CSF,
	//but enough for exactly CSF_MAX_ENTRIES entries.
	//We're assuming we have only one value for one label, which is standard.

	CSFLabel* pLabels;
	GAME_ALLOC_ARR(CSFLabel, CSF_MAX_ENTRIES, pLabels);
	wchar_t** pValues;
	GAME_ALLOC_ARR(wchar_t*, CSF_MAX_ENTRIES, pValues);
	char** pExtraValues;
	GAME_ALLOC_ARR(char*, CSF_MAX_ENTRIES, pExtraValues);

	for(int i = 0; i < CSF_MAX_ENTRIES; i++) {
		*pLabels[i].Name = 0;
		pLabels[i].NumValues = 0;
		pLabels[i].FirstValueIndex = 0;

		pValues[i] = NULL;
		pExtraValues[i] = NULL;
	}

	StringTable::Labels = pLabels;
	StringTable::Values = pValues;
	StringTable::ExtraValues = pExtraValues;

	return 0x7348BC;
}

//0x734A5F, 5
DEFINE_HOOK(734A5F, CSF_AddOrOverrideLabel, 5)
{
	if(CSFLoader::CSFCount > 0)
	{
		CSFLabel* pLabel = (CSFLabel*)bsearch(
			(const char*)0xB1BF38, //label buffer, char[4096]
			StringTable::Labels,
			StringTable::LabelCount,
			sizeof(CSFLabel),
			(int (__cdecl *)(const void *,const void *))_strcmpi);

		if(pLabel)
		{
			//Label already exists - override!

			//If you study the CSF format deeply, you'll call this method suboptimal,
			//because it assumes that we have only one value assigned to one label.
			//This is always the case for RA2, but in no way a limit!
			//Just adding this as a note.
			int idx = pLabel->FirstValueIndex;
			CSFLoader::NextValueIndex = idx;

			wchar_t** pValues = StringTable::Values;
			if(pValues[idx])
			{
				GAME_DEALLOC(pValues[idx]);
				pValues[idx] = NULL;
			}

			char** pExtraValues = StringTable::ExtraValues;
			if(pExtraValues[idx])
			{
				GAME_DEALLOC(pExtraValues[idx]);
				pExtraValues[idx] = NULL;
			}

			auto ix = pLabel - StringTable::Labels;
			R->EBP(ix * sizeof(CSFLabel));
		}
		else
		{
			//Label doesn't exist yet - add!
			int idx = StringTable::ValueCount;
			CSFLoader::NextValueIndex = idx;
			StringTable::ValueCount = idx + 1;
			StringTable::LabelCount = StringTable::LabelCount + 1;

			R->EBP(idx * sizeof(CSFLabel)); //set the index
		}
	}
	return 0;
}

//0x734A97
DEFINE_HOOK(734A97, CSF_SetIndex, 6)
{
	R->EDX(StringTable::Labels);

	if(CSFLoader::CSFCount > 0) {
		R->ECX(CSFLoader::NextValueIndex);
	} else {
		R->ECX(R->Stack32(0x18));
	}

	return 0x734AA1;
}

DEFINE_HOOK(6BD886, CSF_LoadExtraFiles, 5)
{
	CSFLoader::LoadAdditionalCSF("ares.csf");
	char fname[32];
	for(int idx = 0; idx < 100; ++idx) {
		_snprintf(fname, 32, "stringtable%02d.csf", idx);
		CSFLoader::LoadAdditionalCSF(fname);
	}
	R->AL(1);
	return 0x6BD88B;
}

DEFINE_HOOK(734E83, CSF_LoadString_1, 6)
{
	GET(char *, Name, EBX);

	if(strlen(Name) >= 6 && !strncmp(Name, "NOSTR:", 6)) {
		const wchar_t* string = CSFLoader::GetDynamicString(Name, L"%hs", &Name[6]);

		R->EAX(string);

		return 0x734F0F;
	}
	return 0;
}

DEFINE_HOOK(734EC2, CSF_LoadString_2, 7)
{
	GET(char *, Name, EBX);

	const wchar_t* string = CSFLoader::GetDynamicString(Name, L"MISSING:'%hs'", Name);

	R->EAX(string);

	return 0x734F0F;
}

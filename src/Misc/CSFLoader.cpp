#include "CSFLoader.h"
#include <algorithm>
#include <cstdio>

#include "../Ares.h"

int CSFLoader::CSFCount = 0;
int CSFLoader::NextValueIndex = 0;
std::unordered_map<std::string, const CSFString*> CSFLoader::DynamicStrings;

void CSFLoader::LoadAdditionalCSF(const char *pFileName, bool ignoreLanguage)
{
	//The main stringtable must have been loaded (memory allocation)
	//To do that, use StringTable::LoadFile.
	if(StringTable::IsLoaded && pFileName && *pFileName) {
		CCFileClass* pFile = GameCreate<CCFileClass>(pFileName);
		if(pFile->Exists() && pFile->Open(FileAccessMode::Read)) {
			CSFHeader header;

			if(pFile->Read(header)) {
				if(header.Signature == CSF_SIGNATURE &&
					header.CSFVersion >= 2 &&
					(header.Language == StringTable::Language //should stay in one language
						|| header.Language == static_cast<CSFLanguages>(-1)
						|| ignoreLanguage))
				{
					++CSFCount;
					StringTable::ReadFile(pFileName); //must be modified to do the rest ;)

					std::sort(StringTable::Labels, StringTable::Labels + StringTable::LabelCount,
						[](const CSFLabel& lhs, const CSFLabel& rhs)
					{
						return _strcmpi(lhs.Name, rhs.Name) < 0;
					});
				}
			}
		}
		GameDelete(pFile);
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
	return nullptr;
}

const wchar_t* CSFLoader::GetDynamicString(const char* pLabelName, const wchar_t* pPattern, const char* pDefault) {
	const CSFString *String = CSFLoader::FindDynamic(pLabelName);

	if(!String) {
		CSFString* NewString = GameCreate<CSFString>();
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
	//but enough for exactly MaxEntries entries.
	//We're assuming we have only one value for one label, which is standard.

	StringTable::Labels = GameCreateArray<CSFLabel>(CSFLoader::MaxEntries);
	StringTable::Values = GameCreateArray<wchar_t*>(CSFLoader::MaxEntries);
	StringTable::ExtraValues = GameCreateArray<char*>(CSFLoader::MaxEntries);

	return 0x7348BC;
}

//0x734A5F, 5
DEFINE_HOOK(734A5F, CSF_AddOrOverrideLabel, 5)
{
	if(CSFLoader::CSFCount > 0)
	{
		CSFLabel* pLabel = static_cast<CSFLabel*>(bsearch(
			reinterpret_cast<const char*>(0xB1BF38), //label buffer, char[4096]
			StringTable::Labels,
			static_cast<size_t>(StringTable::LabelCount),
			sizeof(CSFLabel),
			(int (__cdecl *)(const void *,const void *))_strcmpi));

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
				GameDelete(pValues[idx]);
				pValues[idx] = nullptr;
			}

			char** pExtraValues = StringTable::ExtraValues;
			if(pExtraValues[idx])
			{
				GameDelete(pExtraValues[idx]);
				pExtraValues[idx] = nullptr;
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
	CSFLoader::LoadAdditionalCSF("ares.csf", true);
	char fname[32];
	for(int idx = 0; idx < 100; ++idx) {
		_snprintf_s(fname, _TRUNCATE, "stringtable%02d.csf", idx);
		CSFLoader::LoadAdditionalCSF(fname);
	}
	R->AL(1);
	return 0x6BD88B;
}

DEFINE_HOOK(734E83, CSF_LoadString_1, 6)
{
	GET(char *, Name, EBX);

	if(!strncmp(Name, "NOSTR:", 6)) {
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

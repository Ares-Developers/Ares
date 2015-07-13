#include <GenericList.h>
#include "Debug.h"
#include "IniSectionIncludes.h"

#include <Helpers\Macro.h>

INIClass::INISection* IniSectionIncludes::includedSection = nullptr;

void IniSectionIncludes::CopySection(CCINIClass* ini, INIClass::INISection* source, const char* destName)
{
	//browse through section entries and copy them over to the new section
	for(GenericNode* node = &source->Entries.First; node; node = node->Next)
	{
		if(*reinterpret_cast<unsigned int*>(node) == 0x7EB734) //type check via vtable address comparison... is there a better way?
		{
			INIClass::INIEntry* entry = static_cast<INIClass::INIEntry*>(node);
			ini->WriteString(destName, entry->Key, entry->Value); //simple but effective
		}
	}
}

CCINIClass::INISection* IniSectionIncludes::PreProcess(CCINIClass* ini, char* str)
{
	char* split = strchr(str, ':');
	if(split)
	{
		//inclusion operator detected, make sure there's a valid section definiton after that
		*split++ = 0;
		if(*split == '[')
		{
			split++;

			char* includedNameEnd = strchr(split, ']');
			if(includedNameEnd && includedNameEnd != split)
			{
				*includedNameEnd = 0;
				return ini->GetSection(split);
			}
		}
	}
	return nullptr;
}

DEFINE_HOOK_AGAIN(525D4D, IniSectionIncludes_PreProcess, 6)
DEFINE_HOOK(525DC1, IniSectionIncludes_PreProcess, 5)
{
	GET_STACK(CCINIClass*, ini, 0x28);
	LEA_STACK(char*, str, 0x78);
	IniSectionIncludes::includedSection = IniSectionIncludes::PreProcess(ini, str);
	return 0;
}

DEFINE_HOOK(525E47, IniSectionIncludes_CopySection1, 6)
{
	if(IniSectionIncludes::includedSection)
	{
		GET_STACK(CCINIClass*, ini, 0x28);
		GET(INIClass::INISection*, section, EBX);
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, section->Name);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}
	return 0;
}

DEFINE_HOOK(525C28, IniSectionIncludes_CopySection2, 0)
{
	LEA_STACK(char*, str, 0x79); //yes, 0x79
	LEA_STACK(char*, sectionName, 0x278)
	strcpy_s(sectionName, 511, str);

	if(IniSectionIncludes::includedSection)
	{
		GET_STACK(CCINIClass*, ini, 0x28);
		IniSectionIncludes::CopySection(ini, IniSectionIncludes::includedSection, sectionName);
		IniSectionIncludes::includedSection = nullptr; //reset, very important
	}

	return 0x525C50;
}

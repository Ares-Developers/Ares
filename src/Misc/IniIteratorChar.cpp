#include "IniIteratorChar.h"

const char IniIteratorChar::iteratorChar[] = "+";
const char IniIteratorChar::iteratorReplacementFormat[] = "%d";

char IniIteratorChar::buffer[0x800] = "\0";

int IniIteratorChar::iteratorValue = 0;

DEFINE_HOOK(5260A2, IteratorChar_Process_Method1, 6)
{
	//GET(CCINIClass*, ini, EBP);
	GET(CCINIClass::INIEntry*, entry, ESI);
	//GET_STACK(CCINIClass::INISection*, section, 0x40);

	if(strcmp(entry->Key, IniIteratorChar::iteratorChar) == 0) {
		sprintf_s(IniIteratorChar::buffer, "%d", IniIteratorChar::iteratorValue++);

		CRT::free(entry->Key);
		entry->Key = CRT::strdup(IniIteratorChar::buffer);
	}

	//debug
	//Debug::Log("[%s] %s = %s (Method 1)\n", section->Name, entry->Key, entry->Value);

	return 0;
}

DEFINE_HOOK(525D23, IteratorChar_Process_Method2, 5)
{
	GET(char*, value, ESI);
	LEA_STACK(char*, key, 0x78)
	//LEA_STACK(char*, section, 0x278);

	if(strcmp(key, IniIteratorChar::iteratorChar) == 0) {
		strcpy(IniIteratorChar::buffer, value);
		sprintf_s(key, 512, "%d", IniIteratorChar::iteratorValue++);

		char* newValue = key + strlen(key) + 1;
		strcpy(newValue, IniIteratorChar::buffer);
		R->ESI<char*>(newValue);
		value = newValue; //for correct debug display
	}

	//Debug::Log("[%s] %s = %s (Method 2)\n", section, key, value);
	return 0;
}

//Uncomment this hook to have all INI sections and their entries printed to the log!
/*
A_FINE_HOOK(474230, IteratorChar_SectionInfo, 5)
{
	GET(CCINIClass*, ini, ESI);

	GenericNode* sectionNode = &ini->Sections.First;
	while(sectionNode)
	{
		if(*((unsigned int*)sectionNode) == 0x7EB73C)
		{
			INIClass::INISection* section = (INIClass::INISection*)sectionNode;
			Debug::Log("[%s]\n", section->Name);

			GenericNode* entryNode = &section->Entries.First;
			while(entryNode)
			{
				if(*((unsigned int*)entryNode) == 0x7EB734)
				{
					INIClass::INIEntry* entry = (INIClass::INIEntry*)entryNode;
					unsigned int checksum = 0xAFFEAFFE;

					for(int i = 0; i < section->CheckedEntries.Count; i++)
					{
						if(section->CheckedEntries.Entries[i].Entry == entry)
						{
							checksum = section->CheckedEntries.Entries[i].Checksum;
							break;
						}
					}
					Debug::Log("\t%s = %s (Checksum: %08X)\n", entry->Key, entry->Value, checksum);
				}
				entryNode = entryNode->Next;
			}
		}
		sectionNode = sectionNode->Next;
	}

	return 0;
}
*/

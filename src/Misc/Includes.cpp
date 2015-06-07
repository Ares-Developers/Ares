#include "../Ares.h"
#include "Includes.h"

int Includes::LastReadIndex = -1;
DynamicVectorClass<CCINIClass*> Includes::LoadedINIs;
DynamicVectorClass<char*> Includes::LoadedINIFiles;

DEFINE_HOOK(474200, CCINIClass_ReadCCFile1, 6)
{
	GET(CCINIClass *, pINI, ECX);
	GET(CCFileClass *, pFile, EAX);

	const char * filename = pFile->GetFileName();

	Includes::LoadedINIs.AddItem(pINI);
	Includes::LoadedINIFiles.AddItem(_strdup(filename));
	return 0;
}

DEFINE_HOOK(474314, CCINIClass_ReadCCFile2, 6)
{
	char buffer[0x80];
	CCINIClass *xINI = Includes::LoadedINIs[Includes::LoadedINIs.Count - 1];

	if(!xINI) {
		return 0;
	}

	const char* section = "#include";

	int len = xINI->GetKeyCount(section);
	for(int i = Includes::LastReadIndex; i < len; i = Includes::LastReadIndex) {
		const char *key = xINI->GetKeyName(section, i);
		++Includes::LastReadIndex;
		buffer[0] = '\0';
		if(xINI->ReadString(section, key, "", buffer)) {
			bool canLoad = true;
			for(int j = 0; j < Includes::LoadedINIFiles.Count; ++j) {
				if(!strcmp(Includes::LoadedINIFiles[j], buffer)) {
					canLoad = false;
					break;
				}
			}

			if(canLoad) {
				CCFileClass *xFile = GameCreate<CCFileClass>(buffer);
				if(xFile->Exists()) {
					xINI->ReadCCFile(xFile);
				}
				GameDelete(xFile);
			}
		}
	}

	Includes::LoadedINIs.RemoveItem(Includes::LoadedINIs.Count - 1);
	if(!Includes::LoadedINIs.Count) {
		for(int j = Includes::LoadedINIFiles.Count - 1; j >= 0; --j) {
			if(char* ptr = Includes::LoadedINIFiles[j]) {
				free(ptr);
			}
			Includes::LoadedINIFiles.RemoveItem(j);
		}
		Includes::LastReadIndex = -1;
	}
	return 0;
}

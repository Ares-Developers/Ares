#include "EVAVoices.h"

#include "Debug.h"
#include "../Ares.CRT.h"

std::vector<const char*> EVAVoices::Types;

// replace the complete ini loading function
DEFINE_HOOK(753000, VoxClass_CreateFromINIList, 6)
{
	GET(CCINIClass*, pINI, ECX);

	char buffer[200];

	// read all EVA types. these are used as additional keys for
	// when the dialogs are read.
	EVAVoices::Types.clear();

	const char* section = "EVATypes";

	if(pINI->GetSection(section)) {
		int count = pINI->GetKeyCount(section);

		for(int i = 0; i < count; ++i) {
			const char* key = pINI->GetKeyName(section, i);
			if(pINI->ReadString(section, key, "", buffer, 200)) {
				EVAVoices::RegisterType(buffer);
			}
		}
	}

	// read all dialogs
	const char* section2 = "DialogList";

	if(pINI->GetSection(section2)) {
		int count = pINI->GetKeyCount(section2);

		for(int i=0; i<count; ++i) {
			const char* key = pINI->GetKeyName(section2, i);
			if(pINI->ReadString(section2, key, "", buffer, 200)) {

				// find or allocate done manually
				VoxClass* pVox = VoxClass::Find(buffer);
				if(!pVox) {
					pVox = new VoxClass2(buffer);
				}

				pVox->LoadFromINI(pINI);
			}
		}
	}

	return 0x75319F;
}

// need to destroy manually because of non-virtual destructor
DEFINE_HOOK(7531CF, VoxClass_DeleteAll, 5)
{
	auto& Array = *VoxClass::Array;

	while(Array.Count) {
		// destroy backwards instead of forwards
		auto pVox = static_cast<VoxClass2*>(Array[Array.Count-1]);
		delete pVox;
	}

	return 0x753240;
}

// also load all additional filenames
DEFINE_HOOK(752FDC, VoxClass_LoadFromINI, 5)
{
	GET(VoxClass2*, pThis, ESI);
	GET(CCINIClass*, pINI, EBP);

	char buffer[0x20];

	// make way for all filenames
	auto count = EVAVoices::Types.size();
	pThis->Voices.resize(count);

	// put the filename in there. 8 chars max.
	for(auto i = 0u; i < count; ++i) {
		pINI->ReadString(pThis->Name, EVAVoices::Types.at(i), "", buffer, 0x20);
		AresCRT::strCopy(pThis->Voices[i].Name, buffer);
	}

	return 0;
}

// undo the inlining. call the function we hook
DEFINE_HOOK(7528E8, VoxClass_PlayEVASideSpecific, 5)
{
	GET(VoxClass*, pVox, EBP);
	const char* ret = pVox->GetFilename();

	R->EDI(ret);
	return 0x752901;
}

// resolve EVA index to filename
DEFINE_HOOK(753380, VoxClass_GetFilename, 5)
{
	GET(VoxClass2*, pThis, ECX);
	int index = VoxClass::EVAIndex;

	const char* ret = nullptr;
	switch(index)
	{
	case -1:
		ret = "";
		break;
	case 0:
		ret = pThis->Allied;
		break;
	case 1:
		ret = pThis->Russian;
		break;
	case 2:
		ret = pThis->Yuri;
		break;
	default:
		ret = pThis->Voices[static_cast<size_t>(index-3)].Name;
	}

	R->EAX(ret);
	return 0x753398;
}

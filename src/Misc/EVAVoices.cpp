#include "EVAVoices.h"

#include "Debug.h"
#include "../Ares.CRT.h"

#include <Helpers\Macro.h>

std::vector<const char*> EVAVoices::Types;

// replace the complete ini loading function
DEFINE_HOOK(753000, VoxClass_CreateFromINIList, 6)
{
	GET(CCINIClass* const, pINI, ECX);

	char buffer[200];

	// read all EVA types. these are used as additional keys for
	// when the dialogs are read.
	EVAVoices::Types.clear();

	auto const pSection = "EVATypes";

	if(pINI->GetSection(pSection)) {
		auto const count = pINI->GetKeyCount(pSection);

		for(auto i = 0; i < count; ++i) {
			auto const pKey = pINI->GetKeyName(pSection, i);
			if(pINI->ReadString(pSection, pKey, "", buffer)) {
				EVAVoices::RegisterType(buffer);
			}
		}
	}

	// read all dialogs
	auto const pSection2 = "DialogList";

	if(pINI->GetSection(pSection2)) {
		auto const count = pINI->GetKeyCount(pSection2);

		for(auto i = 0; i < count; ++i) {
			auto const pKey = pINI->GetKeyName(pSection2, i);
			if(pINI->ReadString(pSection2, pKey, "", buffer)) {

				// find or allocate done manually
				VoxClass* pVox = VoxClass::Find(buffer);
				if(!pVox) {
					pVox = GameCreate<VoxClass2>(buffer);
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
		auto const pVox = static_cast<VoxClass2*>(Array[Array.Count - 1]);
		GameDelete(pVox);
	}

	return 0x753240;
}

// also load all additional filenames
DEFINE_HOOK(752FDC, VoxClass_LoadFromINI, 5)
{
	GET(VoxClass2* const, pThis, ESI);
	GET(CCINIClass* const, pINI, EBP);

	char buffer[0x20];

	// make way for all filenames
	auto const count = EVAVoices::Types.size();
	pThis->Voices.resize(count);

	// put the filename in there. 8 chars max.
	for(auto i = 0u; i < count; ++i) {
		pINI->ReadString(pThis->Name, EVAVoices::Types[i], "", buffer);
		AresCRT::strCopy(pThis->Voices[i].Name, buffer);
	}

	return 0;
}

// undo the inlining. call the function we hook
DEFINE_HOOK(7528E8, VoxClass_PlayEVASideSpecific, 5)
{
	GET(VoxClass* const, pVox, EBP);
	auto const ret = pVox->GetFilename();

	R->EDI(ret);
	return 0x752901;
}

// resolve EVA index to filename
DEFINE_HOOK(753380, VoxClass_GetFilename, 5)
{
	GET(VoxClass2* const, pThis, ECX);
	auto const index = VoxClass::EVAIndex;

	const char* ret = "";
	switch(index)
	{
	case -1:
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
		auto const idxVoc = static_cast<size_t>(index - 3);
		if(idxVoc < pThis->Voices.size()) {
			ret = pThis->Voices[idxVoc].Name;
		}
	}

	R->EAX(ret);
	return 0x753398;
}

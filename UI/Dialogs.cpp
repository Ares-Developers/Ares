/*
	So here's a first example on how to modify the game's dialogs! :3

	To port a dialog from YR to the DLL, add a new .rc file and don't bother editing it
	using MSVC's inbuilt tools.
	Open gamemd.exe in ResHacker, open the selected dialog and copy the whole resource script to
	the new .rc file.

	Don't edit the dialog ID. That would be adding a new dialog and that's a pain to get working
	apparently (I don't know how to do that yet).
	Add #include <windows.h> to the first line of the script.
	If STYLE doesn't list WS_CAPTION, remove the CAPTION "" line.

	Adding new controls is best done in ResHacker, as MSVC wants to add too much stupid
	stuff to things (resource.h and crap like that).
	Also remember to set the control properties like the game originals to avoid problems.
	Finally, be aware that MSVC will automatically add WS_VISIBLE to the controls.
	I don't know why, but I know it sucks, as you'll see when you use the RMG with this version.
	I've yet to find a way to suppress this behavior.

	If you add buttons, they'll look like shit until you register them in the dialog function.

	Adding new dialogs requires many funny hacks I'll try to figure later.

	To make our systems consitent:
	New control IDs within a dialog start with 5000.

	Each dialog code should get its own cpp file to avoid major confusion.
	This central source file contains an addition to YR's "FetchResource"
	that's able to find resources within this DLL as well.
	It's important for it to be just an addition so other DLLs can do this as well!

	Happy GUI modding!
	-pd
*/

#include <Helpers\Macro.h>
#include "../Ares.h"

//4A3B4B, 9 - NOTE: This overrides a call, but it's absolute, so don't worry.
DEFINE_HOOK(4A3B4B, FetchResource, 9)
{
	HMODULE hModule = (HMODULE)Ares::hInstance; //hModule and hInstance are technically the same...
	LPCTSTR lpName = (LPCTSTR)R->get_ECX();
	LPCTSTR lpType = (LPCTSTR)R->get_EDX();

	HRSRC hResInfo = FindResource(hModule, lpName, lpType);

	if(hResInfo)
	{
		HGLOBAL hResData = LoadResource(hModule, hResInfo);
		if(hResData)
		{
			LockResource(hResData);
			R->set_EAX((DWORD)hResData);

			DEBUGLOG("Resource %d loaded successfully: 0x%08X\n", lpName, hResData);

			return 0x4A3B73; //Resource locked and loaded (omg what a pun), return!
		}
	}
	return 0; //Nothing was found, try the game's own resources.
}

#include "Ares.h"
#include "Ares.version.h"
#include "Ares.CRT.h"
//include "CallCenter.h"
#include <StaticInits.cpp>
#include <Unsorted.h>
#include <GetCDClass.h>

#include <new>

#include "Misc/Debug.h"
#include "Misc/EMPulse.h"
#include "Misc/Exception.h"

#ifdef IS_RELEASE_VER
const auto IsStable = true;
#else
const auto IsStable = false;
#endif

//Init Statics
HANDLE Ares::hInstance = 0;
PVOID Ares::pExceptionHandler = nullptr;
Ares::ExceptionHandlerMode Ares::ExceptionMode = Ares::ExceptionHandlerMode::Default;
bool Ares::bNoLogo = false;
bool Ares::bNoCD = false;
bool Ares::bTestingRun = false;
bool Ares::bStrictParser = false;
bool Ares::bAllowAIControl = false;
bool Ares::bFPSCounter = false;
bool const Ares::bStable = IsStable;
bool Ares::bStableNotification = false;
bool Ares::bOutputMissingStrings = false;
bool Ares::bShuttingDown = false;

char Ares::readBuffer[Ares::readLength];
const char Ares::readDelims[4] = ",";
const char Ares::readDefval[4] = "";

MixFileClass *Ares::aresMIX = nullptr;

void Ares::InitOwnResources()
{
	UninitOwnResources();
	aresMIX = GameCreate<MixFileClass>("ares.mix");
}

void Ares::UninitOwnResources()
{
	if(aresMIX) {
		GameDelete(aresMIX);
		aresMIX = nullptr;
	}
}

//Implementations
void __stdcall Ares::CmdLineParse(char** ppArgs, int nNumArgs)
{
	Debug::bLog = false;
	bNoCD = false;
	bNoLogo = false;
	EMPulse::verbose = false;

	// > 1 because the exe path itself counts as an argument, too!
	for(int i = 1; i < nNumArgs; i++) {
		const char* pArg = ppArgs[i];

		if(_stricmp(pArg, "-LOG") == 0) {
			Debug::bLog = true;
		} else if(_stricmp(pArg, "-CD") == 0) {
			bNoCD = true;
		} else if(_stricmp(pArg, "-NOLOGO") == 0) {
			bNoLogo = true;
		} else if(_stricmp(pArg, "-TESTRUN") == 0) {
			bTestingRun = true;
		} else if(_stricmp(pArg, "-STRICT") == 0) {
			bStrictParser = true;
		} else if(_stricmp(pArg, "-LOG-EMP") == 0) {
			EMPulse::verbose = true;
		} else if(_stricmp(pArg, "-AI-CONTROL") == 0) {
			bAllowAIControl = true;
		} else if(_stricmp(pArg, "-LOG-CSF") == 0) {
			bOutputMissingStrings = true;
		} else if(_stricmp(pArg, "-EXCEPTION") == 0) {
			ExceptionMode = ExceptionHandlerMode::NoRemove;
		}
	}

	if(Debug::bLog) {
		Debug::LogFileOpen();
		Debug::Log("Initialized " VERSION_STRVER "\n");
	}

	CheckProcessorFeatures();
	InitNoCDMode();
}

void __stdcall Ares::PostGameInit()
{

}

void __stdcall Ares::ExeRun()
{
	Unsorted::Savegame_Magic = SAVEGAME_MAGIC;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	auto const process = GetCurrentProcess();
	DWORD_PTR const processAffinityMask = 1; // limit to first processor
	SetProcessAffinityMask(process, processAffinityMask);

#if _MSC_VER >= 1700
	// install a new exception handler, if this version of Windows supports it
	if(Ares::ExceptionMode != Ares::ExceptionHandlerMode::Default) {
		if(HINSTANCE handle = GetModuleHandle(TEXT("kernel32.dll"))) {
			if(GetProcAddress(handle, "AddVectoredExceptionHandler")) {
				Ares::pExceptionHandler = AddVectoredExceptionHandler(1, Exception::ExceptionFilter);
			}
		}
	}
#endif
}

void __stdcall Ares::ExeTerminate()
{
	Ares::bShuttingDown = true;

	CloseConfig(Ares::GlobalControls::INI);
	Debug::LogFileClose(111);

	if(Ares::pExceptionHandler && Ares::ExceptionMode != Ares::ExceptionHandlerMode::NoRemove) {
		RemoveVectoredExceptionHandler(Ares::pExceptionHandler);
		Ares::pExceptionHandler = nullptr;
	}
}

CCINIClass* Ares::OpenConfig(const char* file) {
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if(pINI) {
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if(cfg) {
			if(cfg->Exists()) {
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

void Ares::InitNoCDMode() {
	if(!GetCDClass::Instance->Count) {
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		bNoCD = true;
	}
	
	if(bNoCD) {
		Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
		memset(GetCDClass::Instance->Drives, -1, 26);

		char drv[] = "a:\\";
		for(int i=0; i<26; ++i) {
			drv[0] = 'a' + (i + 2) % 26;
			if(GetDriveTypeA(drv) == DRIVE_FIXED) {
				GetCDClass::Instance->Drives[0] = (i + 2) % 26;
				GetCDClass::Instance->Count = 1;
				break;
			}
		}
	}
}

void Ares::CheckProcessorFeatures() {
	BOOL supported = FALSE;
#if _M_IX86_FP == 0 // IA32
	Debug::Log("Ares is not using enhanced instruction set.\n");
#elif _M_IX86_FP == 1 // SSE
#define INSTRUCTION_SET_NAME "SSE"
	supported = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
#elif _M_IX86_FP == 2 && !__AVX__ // SEE2, not AVX
#define INSTRUCTION_SET_NAME "SSE2"
	supported = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
#else // all others, untested. add more #elifs to support more
	static_assert(false, "Ares compiled using unsupported architecture.");
#endif

#ifdef INSTRUCTION_SET_NAME
	Debug::Log("Ares requires a CPU with " INSTRUCTION_SET_NAME " support. %s.\n",
		supported ? "Available" : "Not available");

	if(!supported) {
		MessageBoxA(Game::hWnd,
			"This version of Ares requires a CPU with " INSTRUCTION_SET_NAME
			" support.\n\nYour CPU does not support " INSTRUCTION_SET_NAME ". "
			"Ares will now exit.",
			"Ares - CPU Requirements", MB_ICONERROR);
		Debug::Log("Ares will now exit.\n");
		Ares::ExeTerminate();
		Exception::Exit(553);
	}
#endif
}

void Ares::CloseConfig(CCINIClass* &pINI) {
	if(pINI) {
		GameDelete(pINI);
		pINI = nullptr;
	}
}

//DllMain
bool __stdcall DllMain(HANDLE hInstance,DWORD dwReason,LPVOID v)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			Ares::hInstance = hInstance;
//			Debug::LogFileOpen();
//			Debug::Log("ATTACH\n");
			Debug::LogFileRemove();
			break;
		case DLL_PROCESS_DETACH:
//			Debug::Log("DETACH\n");
//			Debug::LogFileClose();
			break;
	}

	return true;
}

//Exports

//Hook at 0x52C5E0
DEFINE_HOOK(52C5E0, Ares_NoLogo, 7)
{
	return (Ares::bNoLogo)
		? 0x52C5F3u
		: 0u
	;
}

//0x6AD0ED
DEFINE_HOOK(6AD0ED, Ares_AllowSinglePlay, 5)
{
	return 0x6AD16C;
}

/*
	// 55AFB3, 6
A_FINE_HOOK(55AFB3, Armageddon_Advance, 6)
{
	switch(FrameStepCommandClass::ArmageddonState)
	{
		case 1:
			Unsorted::ArmageddonMode = 0;
			FrameStepCommandClass::ArmageddonState = 2;
			break;
		case 2:
			Unsorted::ArmageddonMode = 1;
			FrameStepCommandClass::ArmageddonState = 0;
			break;
		default:
			break;
	}
	return 0;
}
*/

DEFINE_HOOK(7CD810, ExeRun, 9)
{
	Ares::ExeRun();
	return 0;
}

DEFINE_HOOK(7CD8EF, ExeTerminate, 9)
{
	Ares::ExeTerminate();
	GET(UINT, result, EAX);
	ExitProcess(result); //teehee
}

DEFINE_HOOK(52CAE9, _YR_PostGameInit, 5)
{
	Ares::PostGameInit();
	return 0;
}

DEFINE_HOOK(52F639, _YR_CmdLineParse, 5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Ares::CmdLineParse(ppArgs,nNumArgs);
	return 0;
}

/*
A_FINE_HOOK(7C8E17, operator_new, 6)
{
	GET_STACK(size_t, sz, 0x4);

	void * p = operator new(sz, std::nothrow);
	if(!p) {
		Debug::Log("Boom! 0x%X bytes allocated, failed to alloc 0x%X more\n", MemMap::Total, sz);
		exit(1);
	}

	R->EAX(p);
	return 0x7C8E24;
}

A_FINE_HOOK(7C8B3D, operator_delete, 9)
{
	GET_STACK(void *, p, 0x4);
	operator delete(p);
	return 0x7C8B47;
}
*/

DEFINE_HOOK(47AE36, CDFileClass_SetFileName, 8)
{
	GET(void*, CDControl, EAX);

	if(!CDControl || Ares::bNoCD) {
		return 0x47AEF0;
	}
	return 0x47AE3E;
}

DEFINE_HOOK(47B026, FileFindOpen, 8)
{
	GET(void*, CDControl, EBX);

	if(!CDControl || Ares::bNoCD) {
		return 0x47B0AE;
	}
	return 0x47B02E;
}

void Ares::UpdateStability() {
#pragma warning(suppress: 4127) // evaluates to constant
	if(Ares::bStable) {
		return;
	}
	if(Unsorted::CurrentFrame >= 900 && !Ares::bStableNotification) {
		Debug::FatalErrorAndExit(
			"This version of Ares is not considered stable, but for some "
			"reason the warning text isn't showing.\n"
			"This suggests that your version of Ares has been tampered with "
			"and the original developers cannot be held responsible for any "
			"problems you might experience.");
	}
}

wchar_t const* Ares::GetStabilityWarning()
{
#pragma warning(suppress: 4127) // evaluates to constant
	if(!Ares::bStable) {
		return L"This version of Ares (" VERSION_WSTR
			L") is not considered stable.";
	}

	return nullptr;
}

void Ares::SaveGame() {
	Debug::Log("About to save the game\n");
}

void Ares::LoadGame() {
	Debug::Log("About to load the game\n");
}

const DWORD YR_SIZE_1000 = 0x496110;
const DWORD YR_SIZE_1001 = 0x497110;
const DWORD YR_SIZE_1001_UC = 0x497FE0;
const DWORD YR_SIZE_NPATCH = 0x5AB000;

const DWORD YR_TIME_1000 = 0x3B846665;
const DWORD YR_TIME_1001 = 0x3BDF544E;

const DWORD YR_CRC_1000 = 0xB701D792;
const DWORD YR_CRC_1001_CD = 0x098465B3;
const DWORD YR_CRC_1001_TFD = 0xEB903080;
const DWORD YR_CRC_1001_UC = 0x1B499086;

SYRINGE_HANDSHAKE(pInfo)
{
	if(pInfo) {
		const char* AcceptMsg = "Found Yuri's Revenge %s. Applying Ares " PRODUCT_STR ".";
		const char* PatchDetectedMessage = "Found %s. Ares " PRODUCT_STR " is not compatible with other patches.";

		const char* desc = nullptr;
		const char* msg = nullptr;
		bool allowed = false;

		// accept tfd and cd version 1.001
		if(pInfo->exeTimestamp == YR_TIME_1001) {

			// don't accept expanded exes
			switch(pInfo->exeFilesize) {
			case YR_SIZE_1001:
			case YR_SIZE_1001_UC:

				// all versions allowed
				switch(pInfo->exeCRC) {
				case YR_CRC_1001_CD:
					desc = "1.001 (CD)";
					break;
				case YR_CRC_1001_TFD:
					desc = "1.001 (TFD)";
					break;
				case YR_CRC_1001_UC:
					desc = "1.001 (UC)";
					break;
				default:
					// no-cd, network port or other changes
					desc = "1.001 (modified)";
				}
				msg = AcceptMsg;
				allowed = true;
				break;

			case YR_SIZE_NPATCH:
				// known patch size
				desc = "RockPatch or an NPatch-derived patch";
				msg = PatchDetectedMessage;
				break;

			default:
				// expanded exe, unknown make
				desc = "an unknown game patch";
				msg = PatchDetectedMessage;
			}
		} else if(pInfo->exeTimestamp == YR_TIME_1000) {
			// upgrade advice for version 1.000
			desc = "1.000";
			msg = "Found Yuri's Revenge 1.000 but Ares " PRODUCT_STR " requires version 1.001. Please update your copy of Yuri's Revenge first.";
		} else {
			// does not even compute...
			msg = "Unknown executable. Ares " PRODUCT_STR " requires Command & Conquer Yuri's Revenge version 1.001 (gamemd.exe).";
		}

		// generate the output message
		if(pInfo->Message) {
			sprintf_s(pInfo->Message, pInfo->cchMessage, msg, desc);
		}

		return allowed ? S_OK : S_FALSE;
	}

	return E_POINTER;
}

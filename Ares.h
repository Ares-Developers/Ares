#ifndef ARES_H
#define ARES_H

#include <YRPP.h>
#include <hash_map>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

class Ares
{
public:
	//Global Options
	static HANDLE	hInstance;

	static bool		bNoLogo;
	static bool		bNoCD;

	static bool		bLog;
	static FILE*	pLogFile;

	static void LogFile_Open()
	{
		LogFile_Close();
		pLogFile=fopen("DEBUG.TXT","w");
	}
	static void LogFile_Close()
	{
		if(pLogFile)fclose(pLogFile);
		pLogFile=NULL;
	}

	static void (_cdecl* Log)(const char* pFormat,...);

	//Callbacks
	static eMouseEventFlags __stdcall MouseEvent(Point2D*,eMouseEventFlags);
	static void __stdcall CmdLineParse(char**,int);

	static void __stdcall ExeRun();
	static void __stdcall ExeTerminate();

	static void __stdcall PostGameInit();

	static void __stdcall RegisterCommands();

	//General functions
	static void SendPDPlane(
		HouseClass* pOwner,
		CellClass* pDestination,
		AircraftTypeClass* pPlaneType,
		TypeList<TechnoTypeClass*>* pTypes,
		TypeList<int>* pNums);
};

#endif

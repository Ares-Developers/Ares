#ifndef CMD_FPSCOUNTER_H
#define CMD_FPSCOUNTER_H

#include "Ares.h"
#include "../Misc/Debug.h"

class FPSCounterCommandClass : public CommandClass
{
public:
	//Constructor
	FPSCounterCommandClass(){}

	//Destructor
	virtual ~FPSCounterCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "FPS Counter"; }

	virtual const wchar_t* GetUIName()
	{ return L"FPS Counter"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Shows the current and an average of frames per second."; }

	virtual void Execute(DWORD dwUnk)
	{
		Ares::bFPSCounter = !Ares::bFPSCounter;
	}
};

#endif

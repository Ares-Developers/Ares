#ifndef CMD_FRAME_H
#define CMD_FRAME_H

// toggle
class FrameByFrameCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~FrameByFrameCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "SingleStep"; }

	virtual const wchar_t* GetUIName()
		{ return L"Toggle Single-stepping"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Toggles the single-stepping debug mode on or off."; }

	virtual void Execute(DWORD dwUnk)
	{
	 Unsorted::ArmageddonMode = !Unsorted::ArmageddonMode;

		wchar_t msg[0x40] = L"\0";
		wsprintfW(msg, L"Single stepping mode %s.", Unsorted::ArmageddonMode ? L"enabled" : L"disabled");
		MessageListClass::PrintMessage(msg);
	}

	//Constructor
	FrameByFrameCommandClass(){}
};

// step
class FrameStepCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~FrameStepCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "SingleStepForward"; }

	virtual const wchar_t* GetUIName()
	{ return L"Single Step"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Ares"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Proceeds a single step forward."; }

	static int ArmageddonState;

	virtual void Execute(DWORD dwUnk)
	{
		Unsorted::ArmageddonMode = 0;
		ArmageddonState = 1;

		wchar_t msg[0x40] = L"Stepping.";
		MessageListClass::PrintMessage(msg);
	}

	//Constructor
	FrameStepCommandClass(){ }
};

#endif
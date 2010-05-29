#ifndef CMD_MAPSNAP_H
#define CMD_MAPSNAP_H

class MapSnapshotCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~MapSnapshotCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "MapSnapshot"; }

	virtual const wchar_t* GetUIName()
		{ return L"Map Snapshot"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Saves the currently played map."; }

	virtual void Execute(DWORD dwUnk)
	{
		int i = 0;
		
		FILE* F = NULL;
		char buffer[0x10] = "\0";

		do
		{
			if(F)fclose(F);

			_snprintf(buffer, 16, "Map%04d.yrm", i++);
			F = fopen(buffer, "rb");
		}while(F != NULL);

		DEBUGLOG("\t\t%s", buffer);

		char* pBuffer = buffer;

		SET_REG8(dl, 0);
		SET_REG32(ecx, pBuffer);
		CALL(0x687CE0);

		wchar_t msg[0x40] = L"\0";
		wsprintfW(msg, L"Map Snapshot saved as '%hs'.", buffer);
		MessageListClass::PrintMessage(msg);
	}

	//Constructor
	MapSnapshotCommandClass(){}
};

#endif

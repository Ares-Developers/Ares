#include <BuildingTypeClass.h>
#include <MapSeedClass.h>
#include <ScenarioClass.h>
#include <StringTable.h>
#include "Debug.h"
#include "CustomTheater.h"

DynamicVectorClass<CustomTheater*> CustomTheater::Array;

Theater* CustomTheater::FindStock(const char* name)
{
	for(int i = 0; i < 6; i++)
	{
		if(_strcmpi(Theater::Array[i].Identifier, name) == 0)
			return &Theater::Array[i];
	}
	return NULL;
}

CustomTheater* CustomTheater::Get(int i)
{
	if(i >= 0 && i < CustomTheater::Array.Count)
		return CustomTheater::Array[i];
	else
		return NULL;
}

CustomTheater* CustomTheater::Find(const char* name)
{
	for(int i = 0; i < CustomTheater::Array.Count; i++)
	{
		if(_strcmpi(Array[i]->Identifier, name) == 0)
			return Array[i];
	}
	return NULL;
}

int CustomTheater::FindIndex(const char* name)
{
	for(int i = 0; i < CustomTheater::Array.Count; i++)
	{
		if(_strcmpi(Array[i]->Identifier, name) == 0)
			return i;
	}
	return -1;
}

void CustomTheater::LoadFromINIList(CCINIClass* ini)
{
	static char section[] = "Theaters";

	Debug::Log("Initializing theaters...\n");
	if(ini->GetSection(section))
	{
		char buffer[0x10] = "\0";
		size_t bufferSize = 0x10;

		int n = ini->GetKeyCount(section);
		for(int i = 0; i < n; ++i)
		{
			const char* key = ini->GetKeyName(section, i);
			if(ini->ReadString(section, key, "", buffer, bufferSize))
			{
				//check if already exists
				if(!CustomTheater::Find(buffer))
				{
					CustomTheater* theater;
				
					//find stock theater
					Theater* stockTheater = CustomTheater::FindStock(buffer);
					if(stockTheater)
						theater = new CustomTheater(stockTheater);
					else
						theater = new CustomTheater(buffer);
					
					CustomTheater::Array.AddItem(theater);
				}
			}
		}
	}
	else if(Array.Count == 0)
	{
		//add stock theaters
		for(int i = 0; i < 6; i++)
			CustomTheater::Array.AddItem(new CustomTheater(&Theater::Array[i]));
		
		//set defaults for new stuff
		CustomTheater::Array[0]->RMG_Available = true; //temperate
		CustomTheater::Array[1]->RMG_Available = true; //snow
		CustomTheater::Array[3]->RMG_Available = true; //desert
	}
	
	for(int i = 0; i < CustomTheater::Array.Count; i++)
	{
		Debug::Log("\t%s\n", CustomTheater::Array[i]->Identifier);
		CustomTheater::Array[i]->LoadFromINI(ini);
	}
}

void CustomTheater::CleanUp()
{
	for(int i = 0; i < CustomTheater::Array.Count; i++)
	{
		if(CustomTheater::Array[i])
			delete CustomTheater::Array[i];
	}
	CustomTheater::Array.Clear();
}

void CustomTheater::ReadIntArray(CCINIClass* ini, const char* section, const char* key, DynamicVectorClass<int>* array)
{
	if(ini->ReadString(section, key, "", Ares::readBuffer, Ares::readLength))
	{
		for(char* x = strtok(Ares::readBuffer, ","); x; x = strtok(NULL, ","))
			array->AddItem(atoi(x));
	}
}

#define READ_ABSTRACTTYPE_ARRAY(ini, section, key, type, array) \
	if(ini ## ->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) \
	{ \
		for(char* x = strtok(Ares::readBuffer, ","); x; x = strtok(NULL, ",")) \
		{ \
			type * p = type ## ::Find(x); \
			if(p) \
				array ## .AddItem(p); \
		} \
	}

void CustomTheater::LoadFromINI(CCINIClass* ini)
{
	char* buffer = Ares::readBuffer;
	size_t bufferSize = Ares::readLength;
	
	if(ini->ReadString(this->Identifier, "UIName", "", buffer, bufferSize))
		strncpy(this->UIName, buffer, 0x0F);
	
	if(ini->ReadString(this->Identifier, "File.Config", "", buffer, bufferSize))
		strncpy(this->ControlFileName, buffer, 0x09);
	
	if(ini->ReadString(this->Identifier, "File.Mix", "", buffer, bufferSize))
		strncpy(this->ArtFileName, buffer, 0x09);
	
	if(ini->ReadString(this->Identifier, "File.Palette", "", buffer, bufferSize))
		strncpy(this->PaletteFileName, buffer, 0x09);
	
	if(ini->ReadString(this->Identifier, "File.Extension", "", buffer, bufferSize))
		strncpy(this->Extension, buffer, 0x04);
	
	if(ini->ReadString(this->Identifier, "File.MMExtension", "", buffer, bufferSize))
		strncpy(this->MMExtension, buffer, 0x04);

	if(ini->ReadString(this->Identifier, "NewTheaterPrefix", "", buffer, bufferSize))
		strncpy(this->Letter, buffer, 0x01);
	
	this->RadarTerrainBrightness = (float)ini->ReadDouble(this->Identifier, "RadarTerrainBrightness", this->RadarTerrainBrightness);
	
	//for experiments
	this->unknown_float_5C = (float)ini->ReadDouble(this->Identifier, "unknown5C", this->unknown_float_5C);
	this->unknown_float_60 = (float)ini->ReadDouble(this->Identifier, "unknown60", this->unknown_float_60);
	this->unknown_float_64 = (float)ini->ReadDouble(this->Identifier, "unknown64", this->unknown_float_64);

	this->unknown_int_68 = ini->ReadInteger(this->Identifier, "unknown68", this->unknown_int_68);
	this->unknown_int_6C = ini->ReadInteger(this->Identifier, "unknown6C", this->unknown_int_6C);
	
	//new stuff
	this->RMG_Available = ini->ReadBool(this->Identifier, "RMG.Available", this->RMG_Available);
	if(this->RMG_Available)
	{
		ReadIntArray(ini, this->Identifier, "RMG.AmbientLight", &this->RMG_AmbientLight);
		ReadIntArray(ini, this->Identifier, "RMG.AmbientRed", &this->RMG_AmbientRed);
		ReadIntArray(ini, this->Identifier, "RMG.AmbientGreen", &this->RMG_AmbientGreen);
		ReadIntArray(ini, this->Identifier, "RMG.AmbientBlue", &this->RMG_AmbientBlue);
		READ_ABSTRACTTYPE_ARRAY(ini, this->Identifier, "RMG.OrePatchLamps", BuildingTypeClass, this->RMG_OrePatchLamps);
	}
}

#undef READ_ABSTRACTTYPE_ARRAY

DEFINE_HOOK(48DBE0, CustomTheater_FindIndex, 0)
{
	GET(char*, name, ECX);
	R->EAX<int>(CustomTheater::FindIndex(name));
	return 0x48DC12;
}

#define CURRENT_THEATER (*ScenarioClass::Instance).Theater

//oh lawd
#define POINTER_VOODOO(INDEX, DEST) \
	GET(int, theaterIndex, INDEX); \
	CustomTheater* theater = CustomTheater::Get(theaterIndex); \
	R-> ## DEST ## <DWORD>((DWORD)theater - (DWORD)Theater::Array);

#define POINTER_VOODOO_EX(INDEX, DEST) \
	CustomTheater* theater = CustomTheater::Get(INDEX); \
	R-> ## DEST ## <DWORD>((DWORD)theater - (DWORD)Theater::Array);

DEFINE_HOOK(5997C0, CustomTheater_Delegate_InitRandomMap, 6) { POINTER_VOODOO(EAX, ECX); return 0; }
DEFINE_HOOK(5452EB, CustomTheater_Delegate_IsoTileTypes, 7) { POINTER_VOODOO(EDI, ESI); return 0; }
DEFINE_HOOK(5349E3, CustomTheater_Delegate_Init, 6) { POINTER_VOODOO(EDI, ESI); return 0; }
DEFINE_HOOK(4279BB, CustomTheater_Delegate_AnimTypes1, 6) { POINTER_VOODOO(EBP, EDX); return 0; }
DEFINE_HOOK(427AF1, CustomTheater_Delegate_AnimTypes2, 6) { POINTER_VOODOO(EDI, EAX); return 0; }
DEFINE_HOOK(45E9FD, CustomTheater_Delegate_BuildingTypes1, 6) { POINTER_VOODOO(EDI, ECX); return 0; }
DEFINE_HOOK(45EA60, CustomTheater_Delegate_BuildingTypes2, 6) { POINTER_VOODOO(EDI, ECX); return 0; }
DEFINE_HOOK(5FE673, CustomTheater_Delegate_OverlayTypes1, 6) { POINTER_VOODOO(EDI, ECX); return 0; }
DEFINE_HOOK(6B54CF, CustomTheater_Delegate_SmudgeTypes1, 6) { POINTER_VOODOO(EDI, ECX); return 0; }
DEFINE_HOOK(71DCE4, CustomTheater_Delegate_TerrainTypes, 6) { POINTER_VOODOO(EBP, ECX); return 0; }
DEFINE_HOOK(5F91D7, CustomTheater_Delegate_ObjectTypes2, 6) { POINTER_VOODOO(ESI, EDX); return 0; }
DEFINE_HOOK(5F96F8, CustomTheater_Delegate_ObjectTypes3, 6) { POINTER_VOODOO(EDX, EAX); return 0; }
DEFINE_HOOK(47C318, CustomTheater_Delegate_Unknown, 6) { POINTER_VOODOO(EAX, EDX); return 0; }

//some need special treatment because a register gets overridden too early, index is explicitly given here
DEFINE_HOOK(5F915C, CustomTheater_Delegate_ObjectTypes, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, EDX); return 0; }
DEFINE_HOOK(428903, CustomTheater_Delegate_AnimTypes3, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, ECX); return 0; }
DEFINE_HOOK(428CBF, CustomTheater_Delegate_AnimTypes4, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, ECX); return 0; }
DEFINE_HOOK(4758D4, CustomTheater_Delegate_WriteTheater, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, EDX); return 0; }
DEFINE_HOOK(5FEB94, CustomTheater_Delegate_OverlayTypes2, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, ECX); return 0; }
DEFINE_HOOK(5FEE42, CustomTheater_Delegate_OverlayTypes3, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, EDX); return 0; }
DEFINE_HOOK(6B57A7, CustomTheater_Delegate_SmudgeTypes2, 6) { POINTER_VOODOO_EX(CURRENT_THEATER, ECX); return 0; }

DEFINE_HOOK(6275F4, CustomTheater_Delegate_PaletteCollection, 6)
{
	GET(int*, indexp, EBP);
	POINTER_VOODOO_EX(*indexp, EDI);
	return 0;
}

//not coverable using my voodoo :(
DEFINE_HOOK(74D45A, CustomTheater_Delegate_Veinhole, 0)
{
	GET(int, index, ECX);
	R->EAX<char*>(CustomTheater::Get(index)->Extension);
	R->ECX<DWORD>(R->ESP());
	return 0x74D468;
}

#undef CURRENT_THEATER
#undef POINTER_VOODOO
#undef POINTER_VOODOO_EX

DEFINE_HOOK(6722F0, CustomTeater_Init, 5)
{
	GET_STACK(CCINIClass*, ini, 0x04);
	CustomTheater::LoadFromINIList(ini);
	return 0;
}

DEFINE_HOOK(6BEAC3, CustomTheater_Clear, 6)
{
	CustomTheater::CleanUp();
	return 0;
}

//RMG stuff

DEFINE_HOOK(5970B1, RMG_CustomTheater_FillTheaterList, 5)
{
	GET(HWND, hWnd, EDI); //listbox handle
	
	const wchar_t* defaultSelection = NULL;
	for(int i = 0; i < CustomTheater::Array.Count; i++)
	{
		if(CustomTheater::Array[i]->RMG_Available)
		{
			const wchar_t* displayName = StringTable::LoadString(CustomTheater::Array[i]->UIName);
			
			if(!defaultSelection)
				defaultSelection = displayName;
		
			//list the item
			LRESULT result = SendMessageA(hWnd, WW_CB_ADDITEM, 0, (LPARAM)displayName); //text display
			SendMessageA(hWnd, CB_SETITEMDATA, result, i); //set connected theater index
		}
	}
	
	//set default selection
	LRESULT result =  SendMessageA(hWnd, 0x4BE, 0, (LPARAM)defaultSelection); //not sure, maybe sets the default text?
	SendMessageA(hWnd, CB_SETCURSEL, result, 0); //sets default selected item
	
	GET_STACK(MapSeedClass*, mapSeed, 0x10);
	R->EBP<MapSeedClass*>(mapSeed);
	return 0x59712A;
}

DEFINE_HOOK(599863, RMG_CustomTheater_AmbientLight, 5)
{
	GET(MapSeedClass*, mapSeed, EDI);
	
	CustomTheater* theater = CustomTheater::Get(mapSeed->Theater);
	if(theater && theater->RMG_AmbientLight.Count >= 4)
	{
		R->EDX<int>(theater->RMG_AmbientLight[mapSeed->Time]);
		return 0x59987E;
	}
	else
	{
		return 0;
	}
}

DEFINE_HOOK(59A56B, RMG_CustomTheater_AmbientLightColor, 5)
{
	GET(MapSeedClass*, mapSeed, EDI);
	
	CustomTheater* theater = CustomTheater::Get(mapSeed->Theater);
	if(theater &&
		theater->RMG_AmbientRed.Count >= 4 &&
		theater->RMG_AmbientGreen.Count >= 4 &&
		theater->RMG_AmbientBlue.Count >= 4
	)
	{
		ScenarioClass& scenario = *ScenarioClass::Instance;
		int time = mapSeed->Time;
		
		scenario.Red = theater->RMG_AmbientRed[time];
		scenario.Green = theater->RMG_AmbientGreen[time];
		scenario.Blue = theater->RMG_AmbientBlue[time];
		
		return 0x59A5F2;
	}
	else
	{
		return 0;
	}
}

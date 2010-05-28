#ifndef CUSTOMTHEATER_H
#define CUSTOMTHEATER_H

#include "../Ares.h"
#include "../Utilities/Template.h"
#include <Theater.h>

class BuildingTypeClass;

/*
	Finally, the custom theaters feature has made it. :)
	
	Now for a rough concept:
	All theater data references will be replaced by this new logic.
	This class will hold a global theater array with all registered theaters, plus extra information.
	
	After rulesmd.ini has been initialized, Ares will try and find the [Theaters] section.
	If it exists, only the theaters listed in that section will be available, if not, the stock theaters will be registered.
	
	If the list contains a theater ID that already comes with the game, it will be auto-configured before the respective ini section is read.
	
	TODO:
	- Hook loading and unloading in RulesExt.
	- Are these savegame relevant?
	
	~pd
*/
class CustomTheater : public Theater
{
private:
	static Theater* FindStock(const char* name);

public:
	static DynamicVectorClass<CustomTheater*> Array;
	
	static CustomTheater* Get(int i);
	static CustomTheater* Find(const char* name);
	static int FindIndex(const char* name);
	
	static void LoadFromINIList(CCINIClass* ini); //TODO: should be invoked by RulesExt
	static void CleanUp(); //TODO: needs to be invoked from somewhere

private:
	void ReadIntArray(CCINIClass* ini, const char* section, const char* key, DynamicVectorClass<int>* array);

	CustomTheater(const char* id)
	{
		strncpy(this->Identifier, id, 0x09);

		//Temperate defaults
		this->RadarTerrainBrightness = 1.0f;
		this->unknown_float_5C = 1.6f;
		this->unknown_float_60 = 1.0f;
		this->unknown_float_64 = 0.0f;
		this->unknown_int_68 = -1;
		this->unknown_int_6C = 0;
	}
	
	CustomTheater(Theater* base)
	{
		strcpy(this->Identifier, base->Identifier);
		strcpy(this->UIName, base->UIName);
		strcpy(this->ControlFileName, base->ControlFileName);
		strcpy(this->ArtFileName, base->ArtFileName);
		strcpy(this->PaletteFileName, base->PaletteFileName);
		strcpy(this->Extension, base->Extension);
		strcpy(this->MMExtension, base->MMExtension);
		strcpy(this->Letter, base->Letter);
		
		this->RadarTerrainBrightness = base->RadarTerrainBrightness;
		this->unknown_float_5C = base->unknown_float_5C;
		this->unknown_float_60 = base->unknown_float_60;
		this->unknown_float_64 = base->unknown_float_64;
		this->unknown_int_68 = base->unknown_int_68;
		this->unknown_int_6C = base->unknown_int_6C;
	}
	
public:
	//RMG
	bool RMG_Available;
	DynamicVectorClass<int> RMG_AmbientLight;
	DynamicVectorClass<int> RMG_AmbientRed;
	DynamicVectorClass<int> RMG_AmbientGreen;
	DynamicVectorClass<int> RMG_AmbientBlue;
	DynamicVectorClass<BuildingTypeClass*> RMG_OrePatchLamps;
	
	void LoadFromINI(CCINIClass* ini);
};

#endif

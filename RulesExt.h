#ifndef RULES_EXT_H
#define RULES_EXT_H

#define RULESEXT_VALIDATION 0xA1B2C3D4

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <hash_map>

#include <CCINIClass.h>
#include <WeaponTypeClass.h>

//ifdef DEBUGBUILD
#include "Debug.h"
//endif

class RulesClassExt
{
	public:

	class Struct
	{
	public:
		DWORD SavegameValidation;
		bool IsInitialized;
		void Initialize();

		Struct()
		{
			SavegameValidation = RULESEXT_VALIDATION;
			IsInitialized = false;
		}
	};

private:
	static Struct Data;

public:
	static Struct* Global()
	{
		return &Data;
	};

};

#endif

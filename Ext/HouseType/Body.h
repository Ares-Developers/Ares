#ifndef HTEXT_H
#define HTEXT_H

#include <CCINIClass.h>
#include <HouseTypeClass.h>
#include <BuildingTypeClass.h>
#include <PCX.h>
#include <StringTable.h>
#include <Helpers\Macro.h>

#include "..\_Container.hpp"

class HouseTypeExt
{
	public:
	typedef HouseTypeClass TT;
	
	class ExtData : public Extension<TT> 
	{
		public:
			char FlagFile[0x20]; //Flag
			char LSFile[0x20]; //LoadScreen
			char LSPALFile[0x20]; //LoadScreen palette
			char TauntFile[0x20]; //Taunt filename format (should contain %d !!!)
			char LSName[0x20]; //Stringtable label
			char LSSpecialName[0x20]; //Stringtable label for this country's special weapon
			char LSBrief[0x20]; //Stringtable label for this country's load brief
			char StatusText[0x20]; //Stringtable label for this country's Skirmish STT
			int RandomSelectionWeight; //This country gets added this many times into the list of legible countries for random selection.
	
			DynamicVectorClass<BuildingTypeClass *> Powerplants;

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
				RandomSelectionWeight (0)
			{
				*FlagFile = 0;
				*LSFile = 0;
				*LSPALFile = 0;
				*TauntFile = 0;
				*LSName = 0;
				*LSSpecialName = 0;
				*LSBrief = 0;
				*StatusText = 0;
			};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadFromRulesFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void Initialize(TT *pThis);
	};

	static Container<HouseTypeExt> ExtMap;

	static int PickRandomCountry();
};

#endif

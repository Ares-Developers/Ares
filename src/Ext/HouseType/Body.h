#ifndef HTEXT_H
#define HTEXT_H

#include <CCINIClass.h>
#include <HouseTypeClass.h>
#include <BuildingTypeClass.h>
#include <PCX.h>
#include <StringTable.h>
#include <ColorScheme.h>

#include "../../Utilities/Template.h"
#include "../../Ares.CRT.h"
#include "../_Container.hpp"

class HouseTypeExt
{
	public:
	typedef HouseTypeClass TT;

	class ExtData : public Extension<TT>
	{
		public:
			enum { ObserverBackgroundWidth = 121, ObserverBackgroundHeight = 96 };

			enum { ObserverFlagPCXX = 70, ObserverFlagPCXY = 70 };
			enum { ObserverFlagPCXWidth = 45, ObserverFlagPCXHeight = 21 };

			char FlagFile[0x20]; //Flag
			char LSFile[0x20]; //LoadScreen
			char LSPALFile[0x20]; //LoadScreen palette
			char TauntFile[0x20]; //Taunt filename format (should contain %d !!!)
			char LSName[0x20]; //Stringtable label
			char LSSpecialName[0x20]; //Stringtable label for this country's special weapon
			char LSBrief[0x20]; //Stringtable label for this country's load brief
			char StatusText[0x20]; //Stringtable label for this country's Skirmish STT
			ColorScheme* LoadTextColor; //The text color used for non-Campaign modes
			int RandomSelectionWeight; //This country gets added this many times into the list of legible countries for random selection.
			int CountryListIndex; //The index this country will appear in the selection list.

			DynamicVectorClass<BuildingTypeClass *> Powerplants;
			TypeList<TechnoTypeClass*> ParaDrop;
			TypeList<int> ParaDropNum;
			ValueableIdx<AircraftTypeClass> ParaDropPlane;
			Valueable<AnimTypeClass*> Parachute_Anim;

			char ObserverBackground[0x20];
			SHPStruct *ObserverBackgroundSHP;

			char ObserverFlag[0x20];
			SHPStruct *ObserverFlagSHP;
			Valueable<bool> ObserverFlagYuriPAL;
			bool SettingsInherited;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
				RandomSelectionWeight (0),
				CountryListIndex (0),
				ParaDropPlane (-1),
				Parachute_Anim (NULL),
				LoadTextColor (NULL),
				ObserverBackgroundSHP (NULL),
				ObserverFlagSHP (NULL),
				ObserverFlagYuriPAL (false),
				SettingsInherited (false)
			{
				*FlagFile = 0;
				*LSFile = 0;
				*LSPALFile = 0;
				*TauntFile = 0;
				*LSName = 0;
				*LSSpecialName = 0;
				*LSBrief = 0;
				*StatusText = 0;
				*ObserverBackground = 0;
				*ObserverFlag = 0;
			};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadFromRulesFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr) {
		}

		AircraftTypeClass* GetParadropPlane();
		bool GetParadropContent(TypeList<TechnoTypeClass*>**, TypeList<int>**);
		AnimTypeClass* GetParachuteAnim();

		void InheritSettings(HouseTypeClass *pThis);
	};

	static Container<HouseTypeExt> ExtMap;

	static int PickRandomCountry();
};

#endif

#ifndef HTEXT_H
#define HTEXT_H

#include <CCINIClass.h>
#include <HouseTypeClass.h>

#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"
#include "../../Ares.CRT.h"
#include "../_Container.hpp"

class AircraftTypeClass;
class AnimTypeClass;
class ColorScheme;
class BuildingTypeClass;
class TechnoTypeClass;

class HouseTypeExt
{
	public:
	typedef HouseTypeClass TT;

	class ExtData : public Extension<TT>
	{
		public:
			static const int ObserverBackgroundWidth = 121;
			static const int ObserverBackgroundHeight = 96;

			static const int ObserverFlagPCXX = 70;
			static const int ObserverFlagPCXY = 70;
			static const int ObserverFlagPCXWidth = 45;
			static const int ObserverFlagPCXHeight = 21;

			AresPCXFile FlagFile; //Flag
			AresFixedString<0x20> LoadScreenBackground; //LoadScreen
			char LSPALFile[0x20]; //LoadScreen palette
			char TauntFile[0x20]; //Taunt filename format (should contain %d !!!)
			char LSName[0x20]; //Stringtable label
			char LSSpecialName[0x20]; //Stringtable label for this country's special weapon
			char LSBrief[0x20]; //Stringtable label for this country's load brief
			char StatusText[0x20]; //Stringtable label for this country's Skirmish STT
			ValueableIdx<ColorScheme> LoadTextColor; //The text color used for non-Campaign modes
			int RandomSelectionWeight; //This country gets added this many times into the list of legible countries for random selection.
			int CountryListIndex; //The index this country will appear in the selection list.

			ValueableVector<BuildingTypeClass *> Powerplants;
			ValueableVector<TechnoTypeClass*> ParaDropTypes;
			ValueableVector<int> ParaDropNum;
			ValueableIdx<AircraftTypeClass> ParaDropPlane;
			Valueable<AnimTypeClass*> Parachute_Anim;

			ValueableVector<BuildingTypeClass*> VeteranBuildings;

			AresPCXFile ObserverBackground;
			SHPStruct *ObserverBackgroundSHP;

			AresPCXFile ObserverFlag;
			SHPStruct *ObserverFlagSHP;
			Valueable<bool> ObserverFlagYuriPAL;
			bool SettingsInherited;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
				RandomSelectionWeight (0),
				CountryListIndex (0),
				ParaDropPlane (-1),
				Parachute_Anim (nullptr),
				VeteranBuildings (),
				LoadTextColor (-1),
				ObserverBackgroundSHP (nullptr),
				ObserverFlagSHP (nullptr),
				ObserverFlagYuriPAL (false),
				SettingsInherited (false)
			{
				*LSPALFile = 0;
				*TauntFile = 0;
				*LSName = 0;
				*LSSpecialName = 0;
				*LSBrief = 0;
				*StatusText = 0;
			};

		virtual ~ExtData() {

		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadFromRulesFile(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		AircraftTypeClass* GetParadropPlane();
		bool GetParadropContent(Iterator<TechnoTypeClass*>&, Iterator<int>&);
		AnimTypeClass* GetParachuteAnim();

		Iterator<BuildingTypeClass*> GetPowerplants() const;
		Iterator<BuildingTypeClass*> GetDefaultPowerplants() const;

		void InheritSettings(HouseTypeClass *pThis);
	};

	static Container<HouseTypeExt> ExtMap;

	static int PickRandomCountry();
};

#endif

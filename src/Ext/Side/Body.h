#ifndef SIDES_H
#define SIDES_H

#include <CCINIClass.h>
#include <SideClass.h>

#include "../../Ares.h"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"
#include "../../Misc/EVAVoices.h"

class AircraftTypeClass;
class BuildingTypeClass;
class ColorScheme;
class InfantryTypeClass;
class InfantryClass;
class UnitTypeClass;
class VoxClass;

#include "../_Container.hpp"

class SideExt
{
	public:
	typedef SideClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		Nullable<InfantryTypeClass*> Disguise;
		Nullable<InfantryTypeClass*> Crew;
		Nullable<InfantryTypeClass*> Engineer;
		Nullable<InfantryTypeClass*> Technician;
		Nullable<int> SurvivorDivisor;
		NullableVector<BuildingTypeClass*> BaseDefenses;
		NullableVector<int> BaseDefenseCounts;
		NullableVector<TechnoTypeClass*> ParaDropTypes;
		NullableVector<int> ParaDropNum;
		ValueableIdx<AircraftTypeClass> ParaDropPlane;
		Nullable<AnimTypeClass*> Parachute_Anim;
		Valueable<ColorStruct> ToolTipTextColor;
		ValueableIdx<ColorScheme> MessageTextColorIndex;
		int SidebarMixFileIndex;
		bool SidebarYuriFileNames;
		ValueableIdx<EVAVoices> EVAIndex;
		Valueable<UnitTypeClass*> HunterSeeker;

		AresFixedString<0x20> ScoreCampaignBackground;
		AresFixedString<0x20> ScoreCampaignTransition;
		AresFixedString<0x20> ScoreCampaignAnimation;
		AresFixedString<0x20> ScoreCampaignPalette;
		AresFixedString<0x20> ScoreMultiplayBackground;
		AresFixedString<0x20> ScoreMultiplayBars;
		AresFixedString<0x20> ScoreMultiplayPalette;

		AresFixedString<0x20> GraphicalTextImage;
		AresFixedString<0x20> GraphicalTextPalette;

		int ArrayIndex;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			ArrayIndex (-1),
			ParaDropPlane (-1),
			HunterSeeker (nullptr),
			ToolTipTextColor (),
			MessageTextColorIndex (-1),
			EVAIndex (-1)
		{
		};

		virtual ~ExtData() {

		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		int GetSurvivorDivisor() const;
		int GetDefaultSurvivorDivisor() const;

		InfantryTypeClass* GetCrew() const;
		InfantryTypeClass* GetDefaultCrew() const;

		InfantryTypeClass* GetEngineer() const;
		InfantryTypeClass* GetTechnician() const;

		InfantryTypeClass* GetDisguise() const;
		InfantryTypeClass* GetDefaultDisguise() const;

		Iterator<int> GetBaseDefenseCounts() const;
		Iterator<int> GetDefaultBaseDefenseCounts() const;

		Iterator<BuildingTypeClass*> GetBaseDefenses() const;
		Iterator<BuildingTypeClass*> GetDefaultBaseDefenses() const;

		Iterator<TechnoTypeClass*> GetParaDropTypes() const;
		Iterator<InfantryTypeClass*> GetDefaultParaDropTypes() const;

		Iterator<int> GetParaDropNum() const;
		Iterator<int> GetDefaultParaDropNum() const;

		AnimTypeClass* GetParachuteAnim() const;

		const char* GetMultiplayerScoreBarFilename(size_t index) const;
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static Container<SideExt> ExtMap;

	static int CurrentLoadTextColor;

	static UniqueGamePtr<SHPStruct> GraphicalTextImage;
	static UniqueGamePtr<BytePalette> GraphicalTextPalette;
	static UniqueGamePtr<ConvertClass> GraphicalTextConvert;

	static void UpdateGlobalFiles();

	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);

	static SHPStruct* GetGraphicalTextImage();
	static ConvertClass* GetGraphicalTextConvert();
};

#endif

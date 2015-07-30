#pragma once

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
	using base_type = SideClass;

	class ExtData final : public Extension<SideClass>
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
		Valueable<int> SidebarMixFileIndex;
		Valueable<bool> SidebarYuriFileNames;
		ValueableIdx<EVAVoices> EVAIndex;
		Valueable<UnitTypeClass*> HunterSeeker;

		AresFixedString<0x20> ScoreCampaignBackground;
		AresFixedString<0x20> ScoreCampaignTransition;
		AresFixedString<0x20> ScoreCampaignAnimation;
		AresFixedString<0x20> ScoreCampaignPalette;
		AresFixedString<0x20> ScoreMultiplayBackground;
		AresFixedString<0x20> ScoreMultiplayBars;
		AresFixedString<0x20> ScoreMultiplayPalette;

		AresFixedString<0x20> ScoreCampaignThemeUnderPar;
		AresFixedString<0x20> ScoreCampaignThemeOverPar;
		AresFixedString<0x20> ScoreMultiplayThemeWin;
		AresFixedString<0x20> ScoreMultiplayThemeLose;

		AresFixedString<0x20> GraphicalTextImage;
		AresFixedString<0x20> GraphicalTextPalette;

		AresFixedString<0x20> DialogBackgroundImage;
		AresFixedString<0x20> DialogBackgroundPalette;

		int ArrayIndex;

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject),
			ArrayIndex(-1),
			ParaDropPlane(-1),
			HunterSeeker(nullptr),
			ToolTipTextColor(),
			MessageTextColorIndex(-1),
			ScoreCampaignThemeUnderPar("SCORE"),
			ScoreCampaignThemeOverPar("SCORE"),
			ScoreMultiplayThemeWin("SCORE"),
			ScoreMultiplayThemeLose("SCORE"),
			EVAIndex(-1)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

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

		const char* GetMultiplayerScoreBarFilename(unsigned int index) const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SideExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static ExtContainer ExtMap;
	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static int CurrentLoadTextColor;

	static UniqueGamePtr<SHPStruct> GraphicalTextImage;
	static UniqueGamePtr<BytePalette> GraphicalTextPalette;
	static UniqueGamePtr<ConvertClass> GraphicalTextConvert;

	static UniqueGamePtr<SHPStruct> DialogBackgroundImage;
	static UniqueGamePtr<BytePalette> DialogBackgroundPalette;
	static UniqueGamePtr<ConvertClass> DialogBackgroundConvert;

	static void UpdateGlobalFiles();

	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);

	static SHPStruct* GetGraphicalTextImage();
	static ConvertClass* GetGraphicalTextConvert();
};

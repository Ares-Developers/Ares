#ifndef SIDES_H
#define SIDES_H

#include <xcompile.h>
#include <CCINIClass.h>
#include <SideClass.h>
#include <ColorScheme.h>
#include <InfantryTypeClass.h>
#include <InfantryClass.h>
#include <BuildingTypeClass.h>
#include <UnitTypeClass.h>

#include "../../Ares.h"
#include "../../Utilities/Template.h"
#include "../../Misc/EVAVoices.h"

class VoxClass;

#include "../_Container.hpp"

class SideExt
{
	public:
	typedef SideClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		Customizable<InfantryTypeClass*> DefaultDisguise;
		Nullable<InfantryTypeClass*> Crew;
		Nullable<int> SurvivorDivisor;
		TypeList<BuildingTypeClass*> BaseDefenses;
		TypeList<int> BaseDefenseCounts;
		TypeList<InfantryTypeClass*>* ParaDropFallbackTypes;
		TypeList<int>* ParaDropFallbackNum;
		TypeList<TechnoTypeClass*> ParaDrop;
		TypeList<int> ParaDropNum;
		ValueableIdx<AircraftTypeClass> ParaDropPlane;
		Customizable<AnimTypeClass*> Parachute_Anim;
		Valueable<ColorStruct> ToolTipTextColor;
		int MessageTextColorIndex;
		int SidebarMixFileIndex;
		bool SidebarYuriFileNames;
		ValueableIdx<EVAVoices> EVAIndex;

		int ArrayIndex;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			ArrayIndex (-1),
			ParaDropPlane (-1),
			Parachute_Anim (&RulesClass::Instance->Parachute),
			ToolTipTextColor (),
			MessageTextColorIndex (-1),
			EVAIndex (-1)
		{
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		int GetSurvivorDivisor() const;
		int GetDefaultSurvivorDivisor() const;

		InfantryTypeClass* GetCrew() const;
		InfantryTypeClass* GetDefaultCrew() const;
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static Container<SideExt> ExtMap;

	static int CurrentLoadTextColor;

	static DWORD BaseDefenses(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD Disguise(REGISTERS* R, DWORD dwReturnAddress, bool bUseESI);
	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);
};

#endif

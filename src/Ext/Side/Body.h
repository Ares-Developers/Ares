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
		Customizable<AnimTypeClass*> Parachute_Anim;
		Valueable<ColorStruct> ToolTipTextColor;
		int MessageTextColorIndex;
		int SidebarMixFileIndex;
		bool SidebarYuriFileNames;
		ValueableIdx<EVAVoices> EVAIndex;

		int ArrayIndex;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
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
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static Container<SideExt> ExtMap;

	static int CurrentLoadTextColor;

	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);
};

#endif

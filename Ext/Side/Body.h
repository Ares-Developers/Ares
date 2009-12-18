#ifndef SIDES_H
#define SIDES_H

#include <CCINIClass.h>
#include <Helpers/Macro.h>
#include <SideClass.h>
#include <ColorScheme.h>
#include <InfantryTypeClass.h>
#include <InfantryClass.h>
#include <BuildingTypeClass.h>
#include <UnitTypeClass.h>

#include "../../Ares.h"
#include "../../Helpers/Template.h"

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
		Customizable<InfantryTypeClass*> Crew;
		Customizable<int> SurvivorDivisor;
		TypeList<BuildingTypeClass*> BaseDefenses;
		TypeList<int> BaseDefenseCounts;
		ColorScheme* LoadTextColor;
		TypeList<TechnoTypeClass*> ParaDrop;
		TypeList<int> ParaDropNum;
		int SidebarMixFileIndex;
		bool SidebarYuriFileNames;
		char EVATag[0x20];	//TODO

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension(Canary, OwnerObject),
			LoadTextColor (NULL)
		{
			*EVATag = 0;
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
	};

	struct VoxFileNameStruct //need to make this a struct for certain reasons
	{
		char FileName[0x10];

		bool operator == (VoxFileNameStruct &t)
			{ return (_strcmpi(FileName, t.FileName) == 0); }
	};

	//Hacks required in other classes:
	//- TechnoTypeClass (Stolen Tech)
	//- HouseClass (Stolen Tech)
	//- VoxClass (EVA)

	static Container<SideExt> ExtMap;

	static stdext::hash_map<VoxClass*, DynamicVectorClass<VoxFileNameStruct> > EVAFiles;

	static DWORD BaseDefenses(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD Disguise(REGISTERS* R, DWORD dwReturnAddress, bool bUseESI);
	static DWORD LoadTextColor(REGISTERS* R, DWORD dwReturnAddress);
	static DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2);
};

#endif

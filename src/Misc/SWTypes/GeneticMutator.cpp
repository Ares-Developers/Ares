#include "GeneticMutator.h"
#include "../../Ares.h"
#include "../../Ext/Infantry/Body.h"
#include "../../Ext/WarheadType/Body.h"
#include "../../Utilities/Helpers.Alex.h"

bool SW_GeneticMutator::HandlesType(int type)
{
	return (type == SuperWeaponType::GeneticMutator);
}

void SW_GeneticMutator::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to Genetic Mutator values
	pData->SW_AnimHeight = 5;
	pData->SW_Anim = RulesClass::Instance->IonBlast;
	pData->SW_Sound = RulesClass::Instance->GeneticMutatorActivateSound;
	pData->SW_Damage = 10000;

	// defaults depend on MutateExplosion property
	pData->Mutate_Explosion = RulesClass::Instance->MutateExplosion;
	if(pData->Mutate_Explosion) {
		pData->SW_Warhead = &RulesClass::Instance->MutateExplosionWarhead;
		pData->SW_WidthOrRange = 5;
	} else {
		pData->SW_Warhead = &RulesClass::Instance->MutateWarhead;
		pData->SW_WidthOrRange = 3;
		pData->SW_Height = 3;
	}
		
	pData->Mutate_KillNatural = true;
	pData->Mutate_IgnoreCyborg = false;
	pData->Mutate_IgnoreNotHuman = false;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_GeneticMutatorDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_GeneticMutatorReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_GeneticMutatorActivated");
	
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::GeneticMutator;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::GeneticMutator];
}

void SW_GeneticMutator::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Mutate_Explosion.Read(&exINI, section, "Mutate.Explosion");
	pData->Mutate_IgnoreCyborg.Read(&exINI, section, "Mutate.IgnoreCyborg");
	pData->Mutate_IgnoreNotHuman.Read(&exINI, section, "Mutate.IgnoreNotHuman");
	pData->Mutate_KillNatural.Read(&exINI, section, "Mutate.KillNatural");

	// whatever happens, always target everything
	pData->SW_AffectsTarget = pData->SW_AffectsTarget | SuperWeaponTarget::AllTechnos;
}

bool SW_GeneticMutator::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	CoordStruct coords;
	CellClass *Cell = MapClass::Instance->GetCellAt(*pCoords);
	Cell->GetCoordsWithBridge(&coords);
	
	if(pThis->IsCharged) {
		if(pData->Mutate_Explosion) {
			// single shot using cellspread warhead
			MapClass::DamageArea(&coords, pData->SW_Damage, nullptr, pData->SW_Warhead, false, pThis->Owner);
		} else {
			// ranged approach
			auto Mutate = [&](InfantryClass* pInf) -> bool {
				// is this thing affected at all?
				if(!pData->IsHouseAffected(pThis->Owner, pInf->Owner)) {
					return true;
				}

				if(!pData->IsTechnoAffected(pInf)) {
					// even if it makes little sense, we do this.
					// infantry handling is hardcoded and thus
					// this checks water and land cells.
					return true;
				}

				InfantryTypeClass* pType = pInf->Type;

				// quick ways out
				if(pType->Cyborg && pData->Mutate_IgnoreCyborg) {
					return true;
				}

				if(pType->NotHuman && pData->Mutate_IgnoreNotHuman) {
					return true;
				}

				// destroy or mutate
				int damage = pType->Strength;
				bool kill = (pType->Natural && pData->Mutate_KillNatural);
				WarheadTypeClass* pWH = kill
					? RulesClass::Instance->C4Warhead
					: pData->SW_Warhead;

				pInf->ReceiveDamage(&damage, 0, pWH, nullptr, true, false, pThis->Owner);

				return true;
			};

			// find everything in range and mutate it
			Helpers::Alex::DistinctCollector<InfantryClass*> items;
			Helpers::Alex::for_each_in_rect_or_range<InfantryClass>(*pCoords, pData->SW_WidthOrRange, pData->SW_Height, std::ref(items));
			items.for_each(Mutate);
		}
	}

	return true;
}
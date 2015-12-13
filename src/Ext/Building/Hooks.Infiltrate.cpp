#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"

#include <FactoryClass.h>
#include <HouseClass.h>
#include <PCX.h>

/* #633 - spy building infiltration */
// wrapper around the entire function
DEFINE_HOOK(4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass *, EnteredBuilding, ECX);
	GET_STACK(HouseClass *, Enterer, 0x4);

	BuildingExt::ExtData *pBuilding = BuildingExt::ExtMap.Find(EnteredBuilding);

	return (pBuilding->InfiltratedBy(Enterer))
		? 0x4575A2
		: 0
	;
}

// #814: force sidebar repaint for standard spy effects
DEFINE_HOOK_AGAIN(4574D2, BuildingClass_Infiltrate_Standard, 6)
DEFINE_HOOK(457533, BuildingClass_Infiltrate_Standard, 6)
{
	MouseClass::Instance->SidebarNeedsRepaint();
	return R->Origin() + 6;
}

DEFINE_HOOK(43E7B0, BuildingClass_DrawVisible, 5)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);

	auto pType = pThis->Type;
	auto pExt = BuildingTypeExt::ExtMap.Find(pType);

	// helpers (with support for the new spy effect)
	bool bAllied = pThis->Owner->IsAlliedWith(HouseClass::Player);
	bool bReveal = pExt->RevealProduction && pThis->DisplayProductionTo.Contains(HouseClass::Player);

	// show building or house state
	if(pThis->IsSelected && (bAllied || bReveal)) {
		Point2D loc = {pLocation->X - 10, pLocation->Y + 10};
		pThis->DrawExtraInfo(&loc, pLocation, pBounds);
	}

	// display production cameo
	if(pThis->IsSelected && bReveal) {
		auto pFactory = pThis->Factory;
		if(pThis->Owner->ControlledByPlayer()) {
			pFactory = pThis->Owner->GetPrimaryFactory(pType->Factory, pType->Naval, BuildCat::DontCare);
		}

		if(pFactory && pFactory->Object) {
			auto pProdType = pFactory->Object->GetTechnoType();
			auto pProdExt = TechnoTypeExt::ExtMap.Find(pProdType);

			// support for pcx cameos
			if(auto pPCX = pProdExt->CameoPCX.GetSurface()) {
				const int cameoWidth = 60;
				const int cameoHeight = 48;

				RectangleStruct cameoBounds = {0, 0, cameoWidth, cameoHeight};
				RectangleStruct destRect = {pLocation->X - cameoWidth / 2, pLocation->Y - cameoHeight / 2, cameoWidth, cameoHeight};
				RectangleStruct destClip = Drawing::Intersect(&destRect, pBounds, nullptr, nullptr);

				DSurface::Hidden_2->Blit(pBounds, &destClip, pPCX, &cameoBounds, &cameoBounds, true, true);
			} else {
				// old shp cameos, fixed palette
				auto pCameo = pProdType->GetCameo();
				auto pConvert = pProdExt->CameoPal.Convert ? pProdExt->CameoPal.GetConvert() : FileSystem::CAMEO_PAL;
				DSurface::Hidden_2->DrawSHP(pConvert, pCameo, 0, pLocation, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
			}
		}
	}

	return 0x43E8F2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(44161C, BuildingClass_Destroy_OldSpy1, 6)
{
	GET(BuildingClass *, B, ESI);
	B->DisplayProductionTo.Clear();
	BuildingExt::UpdateDisplayTo(B);
	return 0x4416A2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(448312, BuildingClass_ChangeOwnership_OldSpy1, a)
{
	GET(HouseClass *, newOwner, EBX);
	GET(BuildingClass *, B, ESI);

	if(B->DisplayProductionTo.Contains(newOwner)) {
		B->DisplayProductionTo.Remove(newOwner);
		BuildingExt::UpdateDisplayTo(B);
	}
	return 0x4483A0;
}

// if this is a radar, drop the new owner from the bitfield
DEFINE_HOOK(448D95, BuildingClass_ChangeOwnership_OldSpy2, 8)
{
	GET(HouseClass *, newOwner, EDI);
	GET(BuildingClass *, B, ESI);

	if(B->DisplayProductionTo.Contains(newOwner)) {
		B->DisplayProductionTo.Remove(newOwner);
	}

	return 0x448DB9;
}

DEFINE_HOOK(44F7A0, BuildingClass_UpdateDisplayTo, 0)
{
	GET(BuildingClass *, B, ECX);
	BuildingExt::UpdateDisplayTo(B);
	return 0x44F813;
}

DEFINE_HOOK(509303, HouseClass_AllyWith_unused, 0)
{
	GET(HouseClass *, pThis, ESI);
	GET(HouseClass *, pThat, EAX);

	pThis->RadarVisibleTo.Add(pThat);
	return 0x509319;
}

#include "Body.h"
#include "../TechnoType/Body.h"

#include <AlphaShapeClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <TacticalClass.h>
#include <Notifications.h>

AresMap<ObjectClass*, AlphaShapeClass*> TechnoExt::AlphaExt;

// conventions for hashmaps like this:
// the value's CTOR is the only thing allowed to .insert() or [] stuff
// the value's (SD)DTOR is the only thing allowed to .erase() stuff

DEFINE_HOOK(420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass*, pSource, 0x4);
	GET(AlphaShapeClass*, pAlpha, ECX);

	if(auto pOldAlpha = TechnoExt::AlphaExt.get_or_default(pSource)) {
		GameDelete(pOldAlpha);
		// pSource is erased from map
	}
	TechnoExt::AlphaExt[pSource] = pAlpha;
	return 0;
}

DEFINE_HOOK(420A71, AlphaShapeClass_CTOR_Anims, 5)
{
	GET(AlphaShapeClass*, pThis, ESI);
	if(pThis->AttachedTo->WhatAmI() == AnimClass::AbsID) {
		PointerExpiredNotification::NotifyInvalidAnim.Add(pThis);
	}
	return 0;
}

DEFINE_HOOK(421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass*, pAlpha, ECX);
	TechnoExt::AlphaExt.erase(pAlpha->AttachedTo);
	return 0;
}

DEFINE_HOOK(421798, AlphaShapeClass_SDDTOR_Anims, 6)
{
	GET(AlphaShapeClass*, pThis, ESI);
	PointerExpiredNotification::NotifyInvalidAnim.Remove(pThis);
	return 0;
}

DEFINE_HOOK(5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass*, pThis, ESI);
	if(auto pAlpha = TechnoExt::AlphaExt.get_or_default(pThis)) {
		GameDelete(pAlpha);
		// pThis is erased from map
	}
	return 0;
}

void UpdateAlphaShape(ObjectClass* pSource) {
	ObjectTypeClass* pSourceType = pSource->GetType();
	if(!pSourceType) {
		return;
	}

	const SHPStruct* pImage = pSourceType->AlphaImage;
	if(!pImage) {
		return;
	}

	CoordStruct XYZ;

	RectangleStruct *ScreenArea = &TacticalClass::Instance->VisibleArea;
	Point2D off = {ScreenArea->X - (pImage->Width / 2), ScreenArea->Y - (pImage->Height / 2)};
	Point2D xy;

	// for animations attached to the owner object, consider
	// the owner object as source, so the display is refreshed
	// whenever the owner object moves.
	auto pOwner = pSource;
	if(auto pAnim = abstract_cast<AnimClass*>(pSource)) {
		if(pAnim->OwnerObject) {
			pOwner = pAnim->OwnerObject;
		}
	}

	if(auto pFoot = abstract_cast<FootClass*>(pOwner)) {
		if(pFoot->LastMapCoords != pFoot->CurrentMapCoords) {
			// we moved - need to redraw the area we were in
			// alas, we don't have the precise XYZ we were in, only the cell we were last seen in
			// so we need to add the cell's dimensions to the dirty area just in case
			XYZ = CellClass::Cell2Coord(pFoot->LastMapCoords);
			Point2D xyTL, xyBR;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyTL);
			// because the coord systems are different - xyz is x/, y\, xy is x-, y|
			XYZ.X += 256;
			XYZ.Y += 256;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyBR);
			Point2D cellDimensions = xyBR - xyTL;
			xy = xyTL;
			xy.X += cellDimensions.X / 2;
			xy.Y += cellDimensions.Y / 2;
			xy += off;
			RectangleStruct Dirty = {xy.X - ScreenArea->X - cellDimensions.X,
				xy.Y - ScreenArea->Y - cellDimensions.Y, pImage->Width + cellDimensions.X * 2,
				pImage->Height + cellDimensions.Y * 2};
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}

	bool Inactive = pSource->InLimbo;

	if(auto pTechno = abstract_cast<TechnoClass*>(pSource)) {
		Inactive |= pTechno->Deactivated;
	}

	if(auto pBld = abstract_cast<BuildingClass*>(pSource)) {
		if(pBld->GetCurrentMission() != Mission::Construction) {
			Inactive |= !pBld->IsPowerOnline();
		}
	}

	if(Inactive) {
		if(auto pAlpha = TechnoExt::AlphaExt.get_or_default(pSource)) {
			GameDelete(pAlpha);
			// pSource is erased from map
		}
		return;
	}

	if(Unsorted::CurrentFrame % 2) { // lag reduction - don't draw a new alpha every frame
		XYZ = pSource->GetCoords();
		TacticalClass::Instance->CoordsToClient(&XYZ, &xy);
		xy += off;
		++Unsorted::IKnowWhatImDoing;
		GameCreate<AlphaShapeClass>(pSource, xy.X, xy.Y);
		--Unsorted::IKnowWhatImDoing;
		//int Margin = 40;
		RectangleStruct Dirty = {xy.X - ScreenArea->X, xy.Y - ScreenArea->Y, pImage->Width, pImage->Height};
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}
}

DEFINE_HOOK(5F3E70, ObjectClass_Update_AlphaLight, 5)
{
	GET(ObjectClass*, pThis, ECX);
	UpdateAlphaShape(pThis);
	return 0;
}

DEFINE_HOOK(423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if(!pThis->Type->IsFlamingGuy) {
		UpdateAlphaShape(pThis);
	}
	return 0;
}

DEFINE_HOOK(420F75, AlphaLightClass_UpdateScreen_ShouldDraw, 5)
{
	GET(AlphaShapeClass*, pAlpha, ECX);

	bool shouldDraw = !pAlpha->IsObjectGone;

	if(shouldDraw) {
		if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
			auto pData = TechnoExt::ExtMap.Find(pTechno);
			shouldDraw = pData->DrawVisualFX();
		}
	}
	return shouldDraw ? 0x420F80 : 0x42132A;
}

DEFINE_HOOK(4210AC, AlphaLightClass_UpdateScreen_Header, 5)
{
	GET(AlphaShapeClass*, pAlpha, EDX);
	GET(SHPStruct *, pImage, ECX);
	if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		auto pData = TechnoExt::ExtMap.Find(pTechno);
		unsigned int idx = pData->AlphaFrame(pImage);
		R->Stack(0x0, idx);
	}
	return 0;
}

DEFINE_HOOK(4211AC, AlphaLightClass_UpdateScreen_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xDC, 0xB4));
	GET_STACK(SHPStruct*, pImage, STACK_OFFS(0xDC, 0x6C));
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		auto pData = TechnoExt::ExtMap.Find(pTechno);
		unsigned int idx = pData->AlphaFrame(pImage);
		R->Stack(0x0, idx);
	}
	return 0;
}


DEFINE_HOOK(421371, TacticalClass_UpdateAlphasInRectangle_ShouldDraw, 5)
{
	GET(int, AlphaLightIndex, EBX);
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	bool shouldDraw = !pAlpha->IsObjectGone;

	if(shouldDraw) {
		if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
			auto pData = TechnoExt::ExtMap.Find(pTechno);
			shouldDraw = pData->DrawVisualFX();
		}
	}
	return shouldDraw ? 0 : 0x421694;
}

DEFINE_HOOK(42146E, TacticalClass_UpdateAlphasInRectangle_Header, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct*, buffer, EDX);
	GET(SHPStruct*, pImage, EDI);

	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	unsigned int idx = 0;
	if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		auto pData = TechnoExt::ExtMap.Find(pTechno);
		idx = pData->AlphaFrame(pImage);
	}
	R->EAX(pImage->GetFrameBounds(*buffer, idx));
	return 0x421478;
}

DEFINE_HOOK(42152C, TacticalClass_UpdateAlphasInRectangle_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xA4, 0x78));
	GET(SHPStruct*, pImage, ECX);
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if(auto pTechno = abstract_cast<TechnoClass*>(pAlpha->AttachedTo)) {
		auto pData = TechnoExt::ExtMap.Find(pTechno);
		unsigned int idx = pData->AlphaFrame(pImage);
		R->Stack(0x0, idx);
	}
	return 0;
}

DEFINE_HOOK(71944E, TeleportLocomotionClass_ILocomotion_Process, 6)
{
	GET(FootClass*, pObject, ECX);
	GET(CoordStruct*, XYZ, EDX);
	*XYZ = pObject->GetCoords();
	R->EAX<CoordStruct*>(XYZ);

	if(auto pType = pObject->GetTechnoType()) {
		if(auto pImage = pType->AlphaImage) {
			Point2D xy;
			TacticalClass::Instance->CoordsToClient(XYZ, &xy);
			RectangleStruct* ScreenArea = &TacticalClass::Instance->VisibleArea;
			Point2D off = {ScreenArea->X - (pImage->Width / 2), ScreenArea->Y - (pImage->Height / 2)};
			xy += off;
			RectangleStruct Dirty =
			  { xy.X - ScreenArea->X, xy.Y - ScreenArea->Y,
				pImage->Width, pImage->Height};
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}

	return 0x719454;
}

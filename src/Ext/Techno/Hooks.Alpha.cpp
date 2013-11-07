#include "Body.h"
#include "../TechnoType/Body.h"

#include <TacticalClass.h>

hash_AlphaExt TechnoExt::AlphaExt;
// conventions for hashmaps like this:
// the value's CTOR is the only thing allowed to .insert() or [] stuff
// the value's (SD)DTOR is the only thing allowed to .erase() stuff

DEFINE_HOOK(420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass *, O, 0x4);
	GET(AlphaShapeClass *, AS, ECX);
	hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(O);
	if(i != TechnoExt::AlphaExt.end()) {
		delete i->second;
	}
	TechnoExt::AlphaExt[O] = AS;
	return 0;
}

DEFINE_HOOK(421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass *, AS, ECX);
	ObjectClass *O = AS->AttachedTo;
	hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(O);
	if(i != TechnoExt::AlphaExt.end()) {
		TechnoExt::AlphaExt.erase(i);
	}
	return 0;
}

DEFINE_HOOK(5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass *, O, ESI);
	hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(O);
	if(i != TechnoExt::AlphaExt.end()) {
		delete i->second;
	}
	return 0;
}

DEFINE_HOOK(5F3E70, ObjectClass_Update, 5)
{
	GET(ObjectClass *, Source, ECX);
	ObjectTypeClass *SourceType = Source->GetType();
	if(!SourceType) {
		return 0;
	}

	SHPStruct *Alpha = SourceType->AlphaImage;
	if(!Alpha) {
		return 0;
	}

	CoordStruct XYZ;

	RectangleStruct *ScreenArea = &TacticalClass::Instance->VisibleArea;
	Point2D off = { ScreenArea->X - (Alpha->Width / 2), ScreenArea->Y - (Alpha->Height / 2) };
	Point2D xy;

	if(FootClass *Foot = generic_cast<FootClass *>(Source)) {
		if(Foot->LastMapCoords != Foot->CurrentMapCoords) {
			// we moved - need to redraw the area we were in
			// alas, we don't have the precise XYZ we were in, only the cell we were last seen in
			// so we need to add the cell's dimensions to the dirty area just in case
			CellClass::Cell2Coord(&Foot->LastMapCoords, &XYZ);
			Point2D xyTL, xyBR;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyTL);
			// because the coord systems are different - xyz is x/, y\, xy is x-, y|
			XYZ.X += 256;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyBR);
			Point2D cellDimensions = xyBR - xyTL;
			xy = xyTL;
			xy.X += cellDimensions.X / 2;
			xy.Y += cellDimensions.Y / 2;
			xy += off;
			RectangleStruct Dirty =
			  { xy.X - ScreenArea->X - cellDimensions.X, xy.Y - ScreenArea->Y - cellDimensions.Y,
				Alpha->Width + cellDimensions.X * 2, Alpha->Height + cellDimensions.Y * 2 };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, 1);
		}
	}

	bool Inactive = Source->InLimbo;

	if(Source->AbstractFlags & ABSFLAGS_ISTECHNO) {
		Inactive |= reinterpret_cast<TechnoClass *>(Source)->Deactivated;
	}

	if(Source->WhatAmI() == abs_Building && Source->GetCurrentMission() != mission_Construction) {
		Inactive |= !reinterpret_cast<BuildingClass *>(Source)->IsPowerOnline();
	}

	if(Inactive) {
		hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(Source);
		if(i != TechnoExt::AlphaExt.end()) {
			delete i->second;
		}
		return 0;
	}

	if(Unsorted::CurrentFrame % 2) { // lag reduction - don't draw a new alpha every frame
		Source->GetCoords(&XYZ);
		TacticalClass::Instance->CoordsToClient(&XYZ, &xy);
		xy += off;
		++Unsorted::IKnowWhatImDoing;
		AlphaShapeClass *placeholder;
		GAME_ALLOC(AlphaShapeClass, placeholder, Source, xy.X, xy.Y);
		--Unsorted::IKnowWhatImDoing;
		//int Margin = 40;
		RectangleStruct Dirty =
		  { xy.X - ScreenArea->X, xy.Y - ScreenArea->Y,
			Alpha->Width, Alpha->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, 1);
	}

	return 0;
}


DEFINE_HOOK(420F75, AlphaLightClass_UpdateScreen_ShouldDraw, 5)
{
	GET(AlphaShapeClass *, A, ECX);

	bool shouldDraw = !A->IsObjectGone;

	if(shouldDraw) {
		if(TechnoClass* T = generic_cast<TechnoClass *>(A->AttachedTo)) {
			TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(T);
			shouldDraw = pData->DrawVisualFX();
		}
	}
	return shouldDraw ? 0x420F80 : 0x42132A;
}

DEFINE_HOOK(4210AC, AlphaLightClass_UpdateScreen_Header, 5)
{
	GET(AlphaShapeClass *, A, EDX);
	GET(SHPStruct *, Image, ECX);
	if(ObjectClass *O = A->AttachedTo) {
		if(TechnoClass * T = generic_cast<TechnoClass *>(O)) {
			TechnoExt::ExtData * pData = TechnoExt::ExtMap.Find(T);
			unsigned int idx = pData->AlphaFrame(Image);
			R->Stack(0x0, idx);
		}
	}
	return 0;
}

DEFINE_HOOK(4211AC, AlphaLightClass_UpdateScreen_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xDC, 0xB4));
	GET_STACK(SHPStruct *, Image, STACK_OFFS(0xDC, 0x6C));
	AlphaShapeClass *A = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if(ObjectClass *O = A->AttachedTo) {
		if(TechnoClass * T = generic_cast<TechnoClass *>(O)) {
			TechnoExt::ExtData * pData = TechnoExt::ExtMap.Find(T);
			unsigned int idx = pData->AlphaFrame(Image);
			R->Stack(0x0, idx);
		}
	}
	return 0;
}


DEFINE_HOOK(421371, TacticalClass_UpdateAlphasInRectangle_ShouldDraw, 5)
{
	GET(int, AlphaLightIndex, EBX);
	AlphaShapeClass *A = AlphaShapeClass::Array->Items[AlphaLightIndex];

	bool shouldDraw = !A->IsObjectGone;

	if(shouldDraw) {
		if(TechnoClass* T = generic_cast<TechnoClass *>(A->AttachedTo)) {
			TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(T);
			shouldDraw = pData->DrawVisualFX();
		}
	}
	return shouldDraw ? 0 : 0x421694;
}

DEFINE_HOOK(42146E, TacticalClass_UpdateAlphasInRectangle_Header, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct *, buffer, EDX);
	GET(SHPStruct *, Image, EDI);

	AlphaShapeClass *A = AlphaShapeClass::Array->Items[AlphaLightIndex];
	unsigned int idx = 0;
	if(ObjectClass *O = A->AttachedTo) {
		if(TechnoClass * T = generic_cast<TechnoClass *>(O)) {
			TechnoExt::ExtData * pData = TechnoExt::ExtMap.Find(T);
			idx = pData->AlphaFrame(Image);
		}
	}
	R->EAX(Image->GetFrameBounds(buffer, idx));
	return 0x421478;
}

DEFINE_HOOK(42152C, TacticalClass_UpdateAlphasInRectangle_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xA4, 0x78));
	GET(SHPStruct *, Image, ECX);
	AlphaShapeClass *A = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if(ObjectClass *O = A->AttachedTo) {
		if(TechnoClass * T = generic_cast<TechnoClass *>(O)) {
			TechnoExt::ExtData * pData = TechnoExt::ExtMap.Find(T);
			unsigned int idx = pData->AlphaFrame(Image);
			R->Stack(0x0, idx);
		}
	}
	return 0;
}

DEFINE_HOOK(71944E, TeleportLocomotionClass_ILocomotion_Process, 6)
{
	GET(FootClass *, Object, ECX);
	GET(CoordStruct *, XYZ, EDX);
	Object->GetCoords(XYZ);
	R->EAX<CoordStruct *>(XYZ);

	if(TechnoTypeClass *Type = Object->GetTechnoType()) {
		if(SHPStruct *Alpha = Type->AlphaImage) {
			Point2D xy;
			TacticalClass::Instance->CoordsToClient(XYZ, &xy);
			RectangleStruct *ScreenArea = &TacticalClass::Instance->VisibleArea;
			Point2D off = { ScreenArea->X - (Alpha->Width / 2), ScreenArea->Y - (Alpha->Height / 2) };
			xy += off;
			RectangleStruct Dirty =
			  { xy.X - ScreenArea->X, xy.Y - ScreenArea->Y,
				Alpha->Width, Alpha->Height };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, 1);
		}
	}

	return 0x719454;
}

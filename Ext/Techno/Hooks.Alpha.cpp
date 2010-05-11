#include "Body.h"
#include "../TechnoType/Body.h"

#include <TacticalClass.h>

hash_AlphaExt TechnoExt::AlphaExt;

DEFINE_HOOK(420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass *, O, 0x4);
	GET(AlphaShapeClass *, AS, ECX);
	hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(O);
	if(i != TechnoExt::AlphaExt.end()) {
//		TechnoExt::AlphaExt.erase(i);
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

DEFINE_HOOK(5F3D5B, ObjectClass_DTOR, A)
{
	GET(ObjectClass *, O, ESI);
	hash_AlphaExt::iterator i = TechnoExt::AlphaExt.find(O);
	if(i != TechnoExt::AlphaExt.end()) {
		delete i->second;
		TechnoExt::AlphaExt.erase(i);
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
	if(!Alpha || (Unsorted::CurrentFrame % 2)) { // lag reduction
		return 0;
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
			TechnoExt::AlphaExt.erase(i);
		}
		return 0;
	}

	CoordStruct XYZ;
	Point2D xy;
	Source->GetCoords(&XYZ);
	TacticalClass::Instance->CoordsToClient(&XYZ, &xy);
	RectangleStruct *ScreenArea = &TacticalClass::Instance->VisibleArea;
	Point2D off = { ScreenArea->X - (Alpha->Width / 2), ScreenArea->Y - (Alpha->Height / 2) };
	xy += off;
	++Unsorted::IKnowWhatImDoing;
	AlphaShapeClass *placeholder;
	GAME_ALLOC(AlphaShapeClass, placeholder, Source, xy.X, xy.Y);
	--Unsorted::IKnowWhatImDoing;
	int Margin = 40;
	RectangleStruct Dirty =
	  { xy.X - ScreenArea->X - Margin, xy.Y - ScreenArea->Y - Margin,
	    Alpha->Width + 2 * Margin, Alpha->Height + 2 * Margin };
	TacticalClass::Instance->RegisterDirtyArea(Dirty, 1);

	return 0;
}

DEFINE_HOOK(4210AC, Alphas_UpdateAll, 5)
{
	GET(AlphaShapeClass *, A, EDX);
	GET(SHPStruct *, Image, ECX);
	ObjectClass *O = A->AttachedTo;
	if(O && (O->AbstractFlags & ABSFLAGS_ISTECHNO)) {
		int countFrames = Conversions::Int2Highest(Image->Frames);
		TechnoClass *T = reinterpret_cast<TechnoClass *>(O);
		DWORD Facing;
		T->Facing.GetFacing(&Facing);
		WORD F = (WORD)Facing;
		int idx = F >> (16 - countFrames);
		R->Stack(0x0, idx);
	}
	return 0;
}

DEFINE_HOOK(42146E, TacticalClass_UpdateAlphasInRectangle, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct *, buffer, EDX);
	GET(SHPStruct *, Image, EDI);

	AlphaShapeClass *A = AlphaShapeClass::Array->Items[AlphaLightIndex];

	ObjectClass *O = A->AttachedTo;
	int idx = 0;
	if(O && (O->AbstractFlags & ABSFLAGS_ISTECHNO)) {
		int countFrames = Conversions::Int2Highest(Image->Frames);
		TechnoClass *T = reinterpret_cast<TechnoClass *>(O);
		DWORD Facing;
		T->Facing.GetFacing(&Facing);
		WORD F = (WORD)Facing;
		idx = F >> (16 - countFrames);
	}
	R->EAX(Image->GetFrameHeader(buffer, idx));
	return 0x421478;
}

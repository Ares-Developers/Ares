#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"

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

// check before drawing the tooltip
DEFINE_HOOK(43E7EF, BuildingClass_DrawVisible_P1, 5)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::ExtData *pType = BuildingTypeExt::ExtMap.Find(B->Type);
	return (pType->RevealProduction && B->DisplayProductionTo.Contains(HouseClass::Player))
		? 0x43E80E
		: 0x43E832
	;
}

// check before drawing production cameo
DEFINE_HOOK(43E832, BuildingClass_DrawVisible_P2, 6)
{
	GET(BuildingClass *, B, ESI);
	return B->DisplayProductionTo.Contains(HouseClass::Player)
		? 0x43E856
		: 0x43E8EC
	;
}

// fix palette for spied factory production cameo drawing
DEFINE_HOOK(43E8D1, BuildingClass_DrawVisible_P3, 8)
{
	GET(TechnoTypeClass *, Type, EAX);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Type);
	R->EAX<SHPStruct *>(Type->Cameo);
	R->EDX<ConvertClass *>(pData->CameoPal.Convert);
	return 0x43E8DF;
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

DEFINE_HOOK(56757F, MapClass_RevealArea0_DisplayTo, 0)
{
	GET(HouseClass *, pThis, ESI);
	GET(HouseClass *, pThat, EAX);

	return pThis->RadarVisibleTo.Contains(pThat)
		? 0x567597
		: 0x56759D
	;
}

DEFINE_HOOK(567AC1, MapClass_RevealArea1_DisplayTo, 0)
{
	GET(HouseClass *, pThis, EBX);
	GET(HouseClass *, pThat, EAX);

	return pThis->RadarVisibleTo.Contains(pThat)
		? 0x567AD9
		: 0x567ADF
	;
}

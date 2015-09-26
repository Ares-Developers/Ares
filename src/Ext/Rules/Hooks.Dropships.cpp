#include "Body.h"
#include "../../Enum/ArmorTypes.h"
#include <Strsafe.h>
#include <InputManagerClass.h>
#include <WWMouseClass.h>

DEFINE_HOOK(4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	InputManagerClass::Instance->DoSomething();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

DEFINE_HOOK(4B99A2, DropshipLoadout_WriteUnit, 0)
{
	GET(TechnoTypeClass *, pType, ESI);

	GET_STACK(bool, Available, STACK_OFFS(0x164, -0x8));

	LEA_STACK(Point2D *, BaseCoords, STACK_OFFS(0x164, 0x14C));
	LEA_STACK(Point2D *, AltCoords, STACK_OFFS(0x164, 0x144));

	const size_t StringLen = 256;

	wchar_t pName[StringLen];
	wchar_t pArmor[StringLen];
	wchar_t pArmament[StringLen];
	wchar_t pCost[StringLen];

	StringCchPrintfW(pName, StringLen, L"Name: %hs", pType->Name);

	if(Available) {
		StringCchPrintfW(pCost, StringLen, L"Cost: %d", pType->GetCost());
	} else {
		StringCchPrintfW(pCost, StringLen, L"Cost: N/A");
	}

	if(auto pPrimary = pType->Weapon[0].WeaponType) {
		StringCchPrintfW(pArmament, StringLen, L"Armament: %hs", pPrimary->Name);
	} else {
		StringCchPrintfW(pArmament, StringLen, L"Armament: NONE");
	}

	if(const auto& pArmorType = ArmorType::Array[static_cast<unsigned int>(pType->Armor)]) {
		StringCchPrintfW(pArmor, StringLen, L"Armor: %hs", pArmorType->Name);
	} else {
		StringCchPrintfW(pArmor, StringLen, L"Armor: UNKNOWN");
	}

	auto Color = ColorScheme::Find(Available ? "Green" : "Red", 1);

	auto pSurface = DSurface::Hidden;
	RectangleStruct pSurfaceRect;
	pSurface->GetRect(&pSurfaceRect);

	Point2D Coords = *BaseCoords;
	Coords.X += 450;
	Coords.Y += 300;

	Drawing::PrintUnicode(AltCoords, pName, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmament, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pArmor, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	Drawing::PrintUnicode(AltCoords, pCost, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	return 0x4B9BBF;
}


DEFINE_HOOK(4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 0)
{
	GET_STACK(SHPStruct *, pBackground, 0xAC);
	if(pBackground) {
		GameDelete(pBackground);
	}

	LEA_STACK(SHPStruct **, pSwipeAnims, 0x290);

	for(auto i = 0; i < 4; ++i) {
		if(auto pAnim = pSwipeAnims[i]) {
			GameDelete(pAnim);
		}
	}

	return 0x4B9445;
}

#include "Body.h"

#include "../../Misc/Exception.h"

#include <FactoryClass.h>
#include <SuperClass.h>
#include <Networking.h>
#include <BuildingTypeClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>

DynamicVectorClass<CameoDataStruct> RulesExt::TabCameos[4];

template <typename... Args>
void DumpAndExit [[noreturn]] (const char* pMessage, Args... args) {
	//Debug::FullDump();
	//Debug::FatalErrorAndExit(pMessage, std::forward<Args>(args)...);
	Exception::Exit(0xCAE0);
}

void RulesExt::ClearCameos() {
	for(auto& cameos : RulesExt::TabCameos) {
		cameos.Clear();
		cameos.CapacityIncrement = 100;
		cameos.Reserve(100);
	}
}

int IndexOfTab(TabDataStruct * tab) {
	for(auto i = 0; i < 4; ++i) {
		auto TestTab = &MouseClass::Instance->Tabs[i];
		if(TestTab == tab) {
			return i;
		}
	}
	DumpAndExit("Failed to determine tab index of ptr %p\n", tab);
	//return -1; does not return
};

// initializing sidebar
DEFINE_HOOK(6A4EA5, SidebarClass_CTOR_InitCameosList, 6)
{
	RulesExt::ClearCameos();

	return 0;
}

// zeroing in preparation for load
DEFINE_HOOK(6A4FD8, SidebarClass_Load_InitCameosList, 6)
{
	RulesExt::ClearCameos();

	return 0;
}

// set factory for cameo
DEFINE_HOOK(6A61B1, SidebarClass_SetFactoryForObject, 0)
{
	enum { Found = 0x6A6210 , NotFound = 0x6A61E6 };

	GET(int, TabIndex, EAX);
	GET(AbstractType, ItemType, EDI);
	GET(int, ItemIndex, EBP);
	GET_STACK(FactoryClass *, Factory, STACK_OFFS(0xC, -0x4));

	for(auto& cameo : RulesExt::TabCameos[TabIndex]) {
		if(cameo.ItemIndex == ItemIndex && cameo.ItemType == ItemType) {
			cameo.CurrentFactory = Factory;
			auto &Tab = MouseClass::Instance->Tabs[TabIndex];
			Tab.unknown_3C = 1;
			Tab.unknown_3D = 1;
			MouseClass::Instance->RedrawSidebar(0);
			return Found;
		}
	}

	return NotFound;
}

// don't check for 75 cameos in active tab
DEFINE_HOOK(6A63B7, SidebarClass_AddCameo_SkipSizeCheck, 0)
{
	enum { AlreadyExists = 0x6A65FF, NewlyAdded = 0x6A63FD };

	GET_STACK(int, TabIndex, 0x18);
	GET(AbstractType, ItemType, ESI);
	GET(int, ItemIndex, EBP);

	for(auto const& cameo : RulesExt::TabCameos[TabIndex]) {
		if(cameo.ItemIndex == ItemIndex && cameo.ItemType == ItemType) {
			return AlreadyExists;
		}
	}

	R->EDI<TabDataStruct *>(&MouseClass::Instance->Tabs[TabIndex]);

	return NewlyAdded;
}

DEFINE_HOOK(6A8710, TabCameoListClass_AddCameo_ReplaceItAll, 0)
{
	GET(TabDataStruct *, pTab, ECX);
	GET_STACK(AbstractType, ItemType, 0x4);
	GET_STACK(int, ItemIndex, 0x8);

	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(cameos.Count != pTab->CameoCount) {
		DumpAndExit("Unsynchronized cameo counts @ %s: tab #%d, old %d, new %d\n", __FUNCTION__, TabIndex, pTab->CameoCount, cameos.Count);
	}

	CameoDataStruct newCameo(ItemIndex, ItemType);
	if(ItemType == BuildingTypeClass::AbsID) {
		newCameo.IsAlt = ObjectTypeClass::IsBuildCat5(ItemType, ItemIndex);
	}

	//Debug::Log("Adding cameo at tab %d, slot %d of %d: AbsID = %d, Index = %d\n", TabIndex, InsertIndex, cameos.Count, ItemType, ItemIndex);

	if(cameos.AddItem(newCameo)) {
		++pTab->CameoCount;

		auto const old_end = cameos.end() - 1;
		auto const it = std::lower_bound(cameos.begin(), old_end, newCameo);
		std::copy_backward(it, old_end, cameos.end());
		*it = newCameo;
	} else {
		Debug::Log("Adding cameo failed?!\n");
	}

	return 0x6A87E7;
}

// pointer #1
DEFINE_HOOK(6A8D1C, TabSidebarCameoClass_MouseMove_GetCameos1, 0)
{
	GET(int, CameoCount, EAX);

	GET(TabDataStruct *, pTab, EBX);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(cameos.Count != CameoCount) {
		DumpAndExit("Unsynchronized cameo counts @ %s: old %d, new %d\n", __FUNCTION__, CameoCount, cameos.Count);
	}

	if(CameoCount < 1) {
		return 0x6A8D8B;
	}

	R->EDI<CameoDataStruct *>(cameos.Items);

	return 0x6A8D23;
}

// pointer #2
DEFINE_HOOK(6A8DB5, TabSidebarCameoClass_MouseMove_GetCameos2, 0)
{
	GET(int, CameoCount, EAX);

	GET(TabDataStruct *, pTab, EBX);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(cameos.Count != CameoCount) {
		DumpAndExit("Unsynchronized cameo counts @ %s: old %d, new %d\n", __FUNCTION__, CameoCount, cameos.Count);
	}

	if(CameoCount < 1) {
		return 0x6A8F64;
	}

	auto ptr = reinterpret_cast<byte *>(cameos.Items);
	ptr += 0x10;
	R->EBP<byte *>(ptr);

	return 0x6A8DC0;
}

// pointer #3
DEFINE_HOOK(6A8F6C, TabSidebarCameoClass_MouseMove_GetCameos3, 0)
{
	GET(TabDataStruct *, pTab, ESI);
	GET_STACK(int, unused, 0x20);

	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(cameos.Count != pTab->CameoCount) {
		DumpAndExit("Unsynchronized cameo counts @ %s: old %d, new %d\n", __FUNCTION__, pTab->CameoCount, cameos.Count);
	}

	if(pTab->CameoCount < 1) {
		return 0x6A902D;
	}

	auto ptr = reinterpret_cast<byte *>(cameos.Items);
	ptr += 0x1C;
	R->ESI<byte *>(ptr);
	R->EBP<int>(unused);

	return 0x6A8F7C;
}

// don't check for <= 75, pointer
DEFINE_HOOK(6A9304, CameoClass_GetTip_NoLimit, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto ptr = reinterpret_cast<byte *>(&cameos.Items[CameoIndex]);
	ptr -= 0x58;
	R->EAX<byte *>(ptr);

	return 0x6A9316;
}

DEFINE_HOOK(6A9747, TabCameoListClass_Draw_GetCameo1, 0)
{
	GET(int, CameoIndex, ECX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto &Item = cameos.Items[CameoIndex];

	auto ptr = reinterpret_cast<byte *>(&Item);
	ptr -= 0x58;
	R->EAX<byte *>(ptr);
	R->Stack<byte *>(0x30, ptr);

	R->ECX(Item.ItemType);

	return (Item.ItemType == AbstractType::Special)
		? 0x6A9936
		: 0x6A9761
	;
}

DEFINE_HOOK(6A9866, TabCameoListClass_Draw_Test10_1, 0)
{
	GET(int, CameoIndex, ECX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	return (cameos.Items[CameoIndex].unknown_10 == 1)
		? 0x6A9874
		: 0x6A98CF
	;
}

DEFINE_HOOK(6A9886, TabCameoListClass_Draw_Test10_2, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto &Item = cameos.Items[CameoIndex];

	auto ptr = reinterpret_cast<byte *>(&Item);
	ptr += 0x10;
	R->EDI<byte *>(ptr);

	auto dwPtr = reinterpret_cast<DWORD *>(ptr);
	R->EAX<DWORD>(*dwPtr);

	return 0x6A9893;
}

DEFINE_HOOK(6A95C8, TabCameoListClass_Draw_TestF10, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto &Item = cameos.Items[CameoIndex];

	R->EDX<DWORD *>(&Item.unknown_10);

	return 0x6A95D3;
}

DEFINE_HOOK(6A99BE, TabCameoListClass_Draw_BreakDrawLoop, 5)
{
	R->Stack8(0x12, 0);
	return 0x6AA01C;
}

DEFINE_HOOK(6A9B4F, TabCameoListClass_Draw_TestFlashFrame, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	R->EAX(Unsorted::CurrentFrame);

	return (cameos.Items[CameoIndex].FlashEndFrame > Unsorted::CurrentFrame)
		? 0x6A9B67
		: 0x6A9BC5
	;
}

DEFINE_HOOK(6A9EBA, TabCameoListClass_Draw_Test10_3, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	return (cameos.Items[CameoIndex].unknown_10 == 2)
		? 0x6A9ECC
		: 0x6AA01C
	;
}

DEFINE_HOOK(6AAD2F, SidebarClass_ProcessCameoClick_LoadCameoData1, 0)
{
	GET(int, CameoIndex, ESI);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		return 0x6AB94F;
	}

	MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	R->Stack<int>(STACK_OFFS(0xAC, 0x80), CameoIndex);

	auto &Item = cameos.Items[CameoIndex];
	R->Stack<int>(STACK_OFFS(0xAC, 0x98), Item.ItemIndex);
	R->Stack<FactoryClass *>(STACK_OFFS(0xAC, 0x94), Item.CurrentFactory);
	R->Stack<int>(STACK_OFFS(0xAC, 0x88), Item.IsAlt);
	R->EBP(Item.ItemType);

	auto ptr = reinterpret_cast<byte *>(&Item);
	ptr -= 0x58;
	R->EBX<byte *>(ptr);

	return 0x6AAD66;
}

DEFINE_HOOK(6AB0B0, SidebarClass_ProcessCameoClick_LoadCameo2, 0)
{
	GET(int, CameoIndex, ESI);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto &Item = cameos.Items[CameoIndex];

	R->EAX<DWORD *>(&Item.unknown_10);

	return 0x6AB0BE;
}

DEFINE_HOOK(6AB49D, SidebarClass_ProcessCameoClick_FixOffset1, 0)
{
	R->EDI<void *>(nullptr);
	R->ECX<void *>(nullptr);

	return 0x6AB4A4;
}

DEFINE_HOOK(6AB4E8, SidebarClass_ProcessCameoClick_FixOffset2, 0)
{
	GET_STACK(int, idx, 0x14);
	R->ECX<int>(idx);

	R->EDX<void *>(nullptr);

	return 0x6AB4EF;
}

DEFINE_HOOK(6AB577, SidebarClass_ProcessCameoClick_FixOffset3, 0)
{
	GET(int, CameoIndex, ESI);
	GET_STACK(FactoryClass *, SavedFactory, 0x18);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto &Item = cameos.Items[CameoIndex];
	Item.unknown_10 = 1;

	auto Progress = (Item.CurrentFactory)
		? Item.CurrentFactory->GetProgress()
		: 0
	;

	R->EAX<int>(Progress);
	R->EBP<void *>(nullptr);

	if(Item.unknown_10 == 1) { // hi, welcome to dumb ideas
		if(Item.Progress.Value > Progress) {
			Progress = (Progress + Item.Progress.Value) / 2;
		}
	}
	Item.Progress.Value = Progress;

	int Frames = SavedFactory->GetBuildTimeFrames();

	R->EAX<int>(Frames);
	R->ECX<void *>(nullptr);

	return 0x6AB5C6;
}

DEFINE_HOOK(6AB620, SidebarClass_ProcessCameoClick_FixOffset4, 0)
{
	R->ECX<void *>(nullptr);

	return 0x6AB627;
}

DEFINE_HOOK(6AB741, SidebarClass_ProcessCameoClick_FixOffset5, 0)
{
	R->EDX<void *>(nullptr);

	return 0x6AB748;
}

DEFINE_HOOK(6AB802, SidebarClass_ProcessCameoClick_FixOffset6, 0)
{
	GET(int, CameoIndex, EAX);

	auto &cameos = RulesExt::TabCameos[MouseClass::Instance->ActiveTabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	cameos.Items[CameoIndex].unknown_10 = 1;

	return 0x6AB814;
}

DEFINE_HOOK(6AB825, SidebarClass_ProcessCameoClick_FixOffset7, 0)
{
	R->ECX<int>(R->EBP<int>());
	R->EDX<void *>(nullptr);

	return 0x6AB82A;
}

DEFINE_HOOK(6AB920, SidebarClass_ProcessCameoClick_FixOffset8, 0)
{
	R->ECX<void *>(nullptr);

	return 0x6AB927;
}

DEFINE_HOOK(6AB92F, SidebarClass_ProcessCameoClick_FixOffset9, 0)
{
	R->EBX<byte *>(R->EBX<byte *>() + 0x6C);

	return 0x6AB936;
}

DEFINE_HOOK(6ABBCB, TabCameoListClass_AbandonCameosFromFactory_GetPointer1, 0)
{
	GET(int, CameoCount, EAX);

	GET(TabDataStruct *, pTab, ESI);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(CameoCount != cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: old %d, new %d\n", __FUNCTION__, cameos.Count, CameoCount);
	}

	if(CameoCount < 1) {
		return 0x6ABC2F;
	}

	auto ptr = reinterpret_cast<byte *>(cameos.Items);
	ptr += 0xC;
	R->ESI<byte *>(ptr);

	return 0x6ABBD2;
}

// don't limit to 75
DEFINE_HOOK(6AC6D9, MapClass_FlashCameo, 0)
{
	GET(unsigned int, TabIndex, EAX);
	GET(int, ItemIndex, ESI);
	GET_STACK(int, Duration, 0x10);

	auto &cameos = RulesExt::TabCameos[TabIndex];
	for(auto i = 0; i < cameos.Count; ++i) {
		auto &cameo = cameos[i];
		if(cameo.ItemIndex == ItemIndex) {
			cameo.FlashEndFrame = Unsorted::CurrentFrame + Duration;
			break;
		}
	}

	return 0x6AC71A;
}

DEFINE_HOOK(6AA6EA, TabCameoListClass_RecheckCameos_Memcpy, 0)
{
	GET(int, CameoIndex, EAX);

	GET(TabDataStruct *, pTab, EBP);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(CameoIndex >= cameos.Count) {
		DumpAndExit("Bad cameo count @ %s: max %d, request %d\n", __FUNCTION__, cameos.Count, CameoIndex);
	}

	auto ptr = &cameos.Items[CameoIndex];

	R->ESI<CameoDataStruct *>(ptr);
	R->EBX<int>(R->Stack<int>(0x30));
	R->ECX<int>(0xD);

	return 0x6AA6FD;
}

DEFINE_HOOK(6AA711, TabCameoListClass_RecheckCameos_FilterAllowedCameos, 0)
{
	GET(TabDataStruct *, pTab, EBP);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];

	if(cameos.Count != pTab->CameoCount) {
		DumpAndExit("Bad cameo count @ %s: old %d, new %d\n", __FUNCTION__, pTab->CameoCount, cameos.Count);
	}

	GET_STACK(int, StripLength, 0x30);
	GET_STACK(CameoDataStruct *, StripData, 0x1C);

	for(auto ix = cameos.Count; ix > 0; --ix) {
		auto &cameo = cameos[ix - 1];

		auto TechnoType = ObjectTypeClass::GetTechnoType(cameo.ItemType, cameo.ItemIndex);
		bool KeepCameo = false;
		if(TechnoType) {
			auto Factory = TechnoType->FindFactory(true, false, false, HouseClass::Player);
			if(Factory) {
				KeepCameo = !!Factory->Owner->CanBuild(TechnoType, false, true);
			}
		} else {
			auto &Supers = HouseClass::Player->Supers;
			if(Supers.ValidIndex(cameo.ItemIndex)) {
				KeepCameo = Supers[cameo.ItemIndex]->Granted;
			}
		}

		if(!KeepCameo) {
			//Debug::Log("Removing cameo %d from tab %d\n", ix - 1, TabIndex);

			if(cameo.CurrentFactory) {
				NetworkEvent Event;
				Event.FillEvent_ProduceAbandonSuspend(
					HouseClass::Player->ArrayIndex, NetworkEvents::Abandon, cameo.ItemType, cameo.ItemIndex, TechnoType ? TechnoType->Naval : 0
				);
				Networking::AddEvent(&Event);
			}
			if(cameo.ItemType == BuildingTypeClass::AbsID || cameo.ItemType == BuildingClass::AbsID) {
				MouseClass::Instance->CurrentBuilding = nullptr;
				MouseClass::Instance->CurrentBuildingType = nullptr;
				MouseClass::Instance->unknown_11AC = 0xFFFFFFFF;
				MouseClass::Instance->SetActiveFoundation(nullptr);
			}
			if(TechnoType) {
				auto Me = TechnoType->WhatAmI();
				if(HouseClass::Player->GetPrimaryFactory(Me, TechnoType->Naval, BuildCat::DontCare)) {
					NetworkEvent Event;
					Event.FillEvent_ProduceAbandonSuspend(
						HouseClass::Player->ArrayIndex, NetworkEvents::AbandonAll, cameo.ItemType, cameo.ItemIndex, TechnoType->Naval
					);
					Networking::AddEvent(&Event);
				}
			}

			for(auto ixStrip = StripLength; ixStrip > 0; --ixStrip) {
				auto &stripCameo = StripData[ixStrip - 1];
				if(stripCameo == cameo) {
					stripCameo = CameoDataStruct();
				}
			}

			if(!cameos.RemoveItem(ix - 1)) {
				Debug::Log("Removing cameo failed?!\n");
			}
			--pTab->CameoCount;

			R->Stack8(0x17, 1);
		}
	}

	return 0x6AAAB3;
}

DEFINE_HOOK(6AAC10, TabCameoListClass_RecheckCameos_GetPointer, 0)
{
	R->Stack<int>(0x10, R->ECX<int>());

	GET(TabDataStruct *, pTab, EBP);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	R->ECX<CameoDataStruct *>(cameos.Items);

	return 0x6AAC17;
}

DEFINE_HOOK(6A7D4A, MouseClass_RecheckCameos_TrapAlignment, 6)
{
	GET(byte *, pTabData, EDI);

	pTabData -= 0x3C;

	auto pTab = reinterpret_cast<TabDataStruct *>(pTabData);
	auto TabIndex = IndexOfTab(pTab);
	auto &cameos = RulesExt::TabCameos[TabIndex];
	if(cameos.Count != pTab->CameoCount) {
		DumpAndExit("Bad cameo count @ %s: old %d, new %d\n", __FUNCTION__, pTab->CameoCount, cameos.Count);
	}

	return 0;
}

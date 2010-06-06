#include "Body.h"
#include "../Techno/Body.h"

DWORD InfantryExt::EvalBalloonHoverInf(TechnoClass *pThis, DWORD Yes, DWORD No, DWORD Dunno)
{
	if(InfantryClass * Inf = specific_cast<InfantryClass *>(pThis)) {
		if(Inf->Type->DeployToLand) {
			auto pData = TechnoExt::ExtMap.Find(Inf);
			return (pData->InfJumpjet_BalloonHovering)
				? Yes
				: No
			;
		}
		return (Inf->Type->BalloonHover)
			? Yes
			: No
		;
	}
	return Dunno;
}

DWORD InfantryExt::EvalBalloonHoverInf(DWORD Locomotor, DWORD Yes, DWORD No, DWORD Dunno)
{
	Locomotor += 0xC;

	FootClass ** Object = reinterpret_cast<FootClass **>(Locomotor); // pointer voodoo

	return InfantryExt::EvalBalloonHoverInf(*Object, Yes, No, Dunno);
}

/* removing several DeployToLand checks that were for units only */
DEFINE_HOOK(54BDA9, JumpjetLocomotionClass_ProcessState2_DTL_AllowInf, 7)
{
	GET(FootClass *, Foot, EDI);

	enum canHover { Yes = 0x54BDCD, No = 0x54BE62 };

	switch(Foot->WhatAmI()) {
	case InfantryClass::AbsID:
		return (reinterpret_cast<InfantryClass *>(Foot)->Type->DeployToLand)
			? Yes
			: No
		;
	case UnitClass::AbsID:
		return (reinterpret_cast<UnitClass *>(Foot)->Type->IsSimpleDeployer)
			? Yes
			: No
		;
	default:
		return No;
	}
}


DEFINE_HOOK(54C1F6, JumpjetLocomotionClass_ProcessState3_DTL_AllowInf, 8)
{
	GET(UnitClass *, Unit, EDI);

	enum canHover { Yes = 0x54C212, No = 0x54C2DC };

	if(Unit) {
		return (Unit->Type->IsSimpleDeployer)
			? Yes
			: No
		;
	}

	GET(DWORD, Locomotor, ESI);
	Locomotor += 0xC;

	FootClass ** Object = reinterpret_cast<FootClass **>(Locomotor); // pointer voodoo

	if (InfantryClass * Inf = specific_cast<InfantryClass *>(*Object)) {
		if(Inf->Type->DeployToLand) {
			auto pData = TechnoExt::ExtMap.Find(Inf);
			R->EDI<InfantryClass *>(Inf);
			return (pData->InfJumpjet_BalloonHovering)
				? Yes
				: No
			;
		}
	}
	return No;
}

DEFINE_HOOK(54CA27, JumpjetLocomotionClass_ProcessState4_DTL_AllowInf, 5)
{
	GET(FootClass *, Foot, ECX);
	R->EAX<FootClass *>(Foot);
	return 0x54CA5E;
}

/* BalloonHover checks */
DEFINE_HOOK(43C409, BuildingClass_ReceivedRadioCommand_BH_CanEnter, 6)
{
	GET(TechnoClass *, T, EDI);
	return InfantryExt::EvalBalloonHoverInf(T, 0x43C413, 0x43C422);
}

DEFINE_HOOK(4C7608, Networking_RespondToEvent_BH_Idle, 6)
{
	GET(TechnoClass *, T, ESI);
	return InfantryExt::EvalBalloonHoverInf(T, 0x4C7612, 0x4C762A);
}

DEFINE_HOOK(51EDB4, InfantryClass_GetCursorOverObject_BH_CanEnter, 6)
{
	GET(InfantryClass *, T, EDI);
	return InfantryExt::EvalBalloonHoverInf(T, 0x51EDBE, 0x51EDC5);
}

DEFINE_HOOK(522A72, InfantryClass_INotifySink_Notice, 6)
{
	GET(DWORD, Ptr, EDI);
	Ptr -= 8;

	InfantryClass * Inf = reinterpret_cast<InfantryClass *>(Ptr);
	return InfantryExt::EvalBalloonHoverInf(Inf, 0x522A86, 0x522A7C);
}

DEFINE_HOOK(53B25B, PsyDom_Fire_BH, 6)
{
	GET(TechnoClass *, T, ESI);
	return InfantryExt::EvalBalloonHoverInf(T, 0x53B364, 0x53B267);
}

DEFINE_HOOK(53C49F, TechnoClass_CanBePermaMC_BH, 6)
{
	GET(TechnoClass *, T, ESI);
	return InfantryExt::EvalBalloonHoverInf(T, 0x53C4A9, 0x53C4AD);
}

DEFINE_HOOK(54C6AD, JumpjetLocomotionClass_ProcessState4_SkipSomethingForUnit, 5)
{
	R->AL(1);
	return 0x54C6BA;
}

DEFINE_HOOK(54CA3C, JumpjetLocomotionClass_ProcessState4_L1, 6)
{
	Debug::Log("#" str(__LINE__) "\n");
	return 0;
}

DEFINE_HOOK(54B1C3, JumpjetLocomotionClass_ILocomotion_MoveTo_L1, 6)
{
	Debug::Log("MoveTo\n");
	Debug::DumpStack(R, 0xF0);

	GET_STACK(DWORD, Locomotor, 0x14);
	Locomotor += 0x8;

	FootClass ** Object = reinterpret_cast<FootClass **>(Locomotor);
	if(FootClass * O = *Object) {
		if(AbstractClass * D = O->Destination) {
			CoordStruct XYZ;
			D->GetCoords(&XYZ);
			Debug::Log("Destination is object of type %d at (%d, %d, %d)\n", D->WhatAmI(), XYZ.X, XYZ.Y, XYZ.Z);
		}
	}

	return 0;
}

DEFINE_HOOK(54B424, JumpjetLocomotionClass_ILocomotion_MoveTo_L2, 7)
{
	GET(DWORD, Locomotion, ESI);
	GET(FootClass *, F, ECX);
	Debug::Log("Object %s in state %d\n", F->GetType()->ID, *(int*)(Locomotion + 0x4C));
	return 0;
}

DEFINE_HOOK(54B45D, JumpjetLocomotionClass_ILocomotion_MoveTo_L3, 7)
{
	Debug::Log("Object going to state 1\n");
	return 0;
}

DEFINE_HOOK(54BCEE, JumpjetLocomotionClass_ProcessState1_BH, 6)
{
	GET(DWORD, Locomotor, ESI);
	return InfantryExt::EvalBalloonHoverInf(Locomotor, 0x54BD26, 0x54BCF8);
}

DEFINE_HOOK(54BE6D, JumpjetLocomotionClass_ProcessState2_BH, 6)
{
	GET(DWORD, Locomotor, ESI);
	return InfantryExt::EvalBalloonHoverInf(Locomotor, 0x54BE77, 0x54BEA3);
}

DEFINE_HOOK(54C0EB, JumpjetLocomotionClass_ProcessState3_BH, 6)
{
	GET(DWORD, Locomotor, ESI);
	bool Hovering = TechnoExt::IsBalloonHovering(Locomotor);

	R->AL(Hovering);
	return 0x54C0F1;
}

DEFINE_HOOK(6EBE69, sub_6EBAD0_BH_1, a)
{
	GET(FootClass *, F, ESI);
	return TechnoExt::IsBalloonHovering(F)
		? 0x6EBE9C
		: 0x6EBE7D
	;
}

DEFINE_HOOK(6EBEDB, sub_6EBAD0_BH_2, a)
{
	GET(FootClass *, F, ESI);
	return TechnoExt::IsBalloonHovering(F)
		? 0x6EBEEF
		: 0x6EBEFF
	;
}

DEFINE_HOOK(6FC2AD, TechnoClass_GetFireError_BH, 5)
{
	GET(TechnoClass *, T, EBP);
	return TechnoExt::IsBalloonHovering(T)
		? 0x6FC2C2
		: 0x6FC2D2
	;
}

DEFINE_HOOK(70930D, FootClass_CanFightBack_BH, a)
{
	GET(FootClass *, F, ESI);
	return TechnoExt::IsBalloonHovering(F)
		? 0x709321
		: 0x709366
	;
}

DEFINE_HOOK(70B671, TechnoClass_41C, 6)
{
	GET(TechnoClass *, T, ESI);
	return TechnoExt::IsBalloonHovering(T)
		? 0x70B683
		: 0x70BCA4
	;
}

DEFINE_HOOK(54D32E, JumpjetLocomotionClass_54D0F0, 5)
{
	GET(DWORD, Locomotor, ESI);
	return TechnoExt::IsBalloonHovering(Locomotor)
		? 0x54D343
		: 0x54D350
	;
}

/* deploy command */
DEFINE_HOOK(51E462, InfantryClass_GetCursorOverObject_SelfDeploy, 6)
{
	GET(InfantryClass *, Inf, EDI);
	GET(eAction, Act, EBP);

	if(Act = act_Self_Deploy) {
		if(Inf->Type->DeployToLand) {
			return 0x51E458; // always show the deploy cursor when hovering over self
		}
	}
	return 0;
}

DEFINE_HOOK(51F6EB, InfantryClass_Unload_Hovering, 6)
{
	GET(InfantryClass *, Inf, ESI);
	if(Inf->Type->DeployToLand) {
		Debug::Log("Deploying hover!\n");
		auto pData = TechnoExt::ExtMap.Find(Inf);
		pData->InfJumpjet_BalloonHovering = !pData->InfJumpjet_BalloonHovering;
		Inf->ForceMission(mission_Guard);
		Inf->SetDestination(Inf->GetCell(), true);
		R->EAX(1);
		return 0x51F7F5;
	}
	return 0;
}

DEFINE_HOOK(4DAA68, FootClass_Update_Hovering, 6)
{
	GET(FootClass *, pThis, ESI);
	if(InfantryClass * Inf = specific_cast<InfantryClass *>(pThis)) {
		if(Inf->Type->DeployToLand) {
			auto pData = TechnoExt::ExtMap.Find(Inf);
			if(!pData->InfJumpjet_BalloonHovering) {
				return 0x4DAAEE;
			}
		}
	}
	return 0;
}

/* TODO: make it obey the deploy command
A_FINE_HOOK(730CB6, DeployCommandClass_Execute_ToggleHover, 6)
{
	GET(InfantryClass *, Inf, EDI);
	if(Inf && !Inf->Type->DeployToLand) {
		R->ECX<InfantryClass *>(Inf);
		return 0x730CBC;
	}
	return 0x730CE0;
}

A_FINE_HOOK(730B9C, DeployCommandClass_Execute_ToggleHoverToo, 7)
{
	GET(TechnoClass *, T, EDI);
	if(InfantryClass * I = specific_cast<InfantryClass *>(T)) {
		if(I->Type->DeployToLand) {

		}
		return 0x730BA8;
	}
	return 0x730BDC;
}
*/

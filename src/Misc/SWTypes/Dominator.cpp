#include "Dominator.h"
#include "../../Ares.h"
#include "../../Ext/Infantry/Body.h"
#include "../../Ext/WarheadType/Body.h"
#include "../../Utilities/Helpers.Alex.h"

SuperClass* SW_PsychicDominator::CurrentPsyDom = nullptr;

bool SW_PsychicDominator::HandlesType(int type)
{
	return (type == SuperWeaponType::PsychicDominator);
}

SuperWeaponFlags::Value SW_PsychicDominator::Flags()
{
	return SuperWeaponFlags::NoEvent;
}

void SW_PsychicDominator::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to PsychicDominator values
	pData->SW_WidthOrRange = (float)RulesClass::Instance->DominatorCaptureRange;
	pData->SW_Damage = RulesClass::Instance->DominatorDamage;
	pData->SW_Warhead = &RulesClass::Instance->DominatorWarhead;
	pData->SW_ActivationSound = RulesClass::Instance->PsychicDominatorActivateSound;

	pData->Dominator_FirstAnimHeight = 750;
	pData->Dominator_SecondAnimHeight = 0;
	pData->Dominator_FireAtPercentage = RulesClass::Instance->DominatorFireAtPercentage;
	pData->Dominator_Ripple = true;
	pData->Dominator_Capture = true;
	pData->Dominator_CaptureMindControlled = true;
	pData->Dominator_CapturePermaMindControlled = true;
	pData->Dominator_CaptureImmuneToPsionics = false;
	pData->Dominator_PermanentCapture = true;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_PsychicDominatorDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_PsychicDominatorReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_PsychicDominatorActivated");

	pData->Message_Abort = CSFText("Msg:DominatorActive");

	pData->Lighting_Ambient = &ScenarioClass::Instance->DominatorAmbient;
	pData->Lighting_Red = &ScenarioClass::Instance->DominatorRed;
	pData->Lighting_Green = &ScenarioClass::Instance->DominatorGreen;
	pData->Lighting_Blue = &ScenarioClass::Instance->DominatorBlue;

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::PsychicDominator;
	pData->SW_AffectsTarget = SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit;
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::PsychicDominator];
}

void SW_PsychicDominator::LoadFromINI(
	SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Dominator_FirstAnimHeight.Read(&exINI, section, "Dominator.FirstAnimHeight");
	pData->Dominator_SecondAnimHeight.Read(&exINI, section, "Dominator.SecondAnimHeight");
	pData->Dominator_FirstAnim.Parse(&exINI, section, "Dominator.FirstAnim");
	pData->Dominator_SecondAnim.Parse(&exINI, section, "Dominator.SecondAnim");
	pData->Dominator_ControlAnim.Parse(&exINI, section, "Dominator.ControlAnim");
	pData->Dominator_FireAtPercentage.Read(&exINI, section, "Dominator.FireAtPercentage");
	pData->Dominator_Capture.Read(&exINI, section, "Dominator.Capture");
	pData->Dominator_Ripple.Read(&exINI, section, "Dominator.Ripple");
	pData->Dominator_CaptureMindControlled.Read(&exINI, section, "Dominator.CaptureMindControlled");
	pData->Dominator_CapturePermaMindControlled.Read(&exINI, section, "Dominator.CapturePermaMindControlled");
	pData->Dominator_CaptureImmuneToPsionics.Read(&exINI, section, "Dominator.CaptureImmuneToPsionics");
	pData->Dominator_PermanentCapture.Read(&exINI, section, "Dominator.PermanentCapture");
}

bool SW_PsychicDominator::AbortFire(SuperClass* pSW, bool IsPlayer) {
	// be one with Yuri! and only one.
	if(PsyDom::Active()) {
		if(IsPlayer) {
			SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW->Type);
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

bool SW_PsychicDominator::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	if(pThis->IsCharged) {
		// we do not use PsyDom::Start() here. instead, we set a global state and
		// let the state machine take care of everything.
		SW_PsychicDominator::CurrentPsyDom = pThis;
		this->newStateMachine(*pCoords, pThis);
	}

	return true;
}

void PsychicDominatorStateMachine::Update() {
	// waiting. lurking in the shadows.
	if(this->Deferment > 0) {
		if(--this->Deferment) {
			return;
		}
	}

	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(this->Super->Type);

	switch(PsyDom::Status) {
	case PsychicDominatorStatus::FirstAnim:
		{
			// here are the contents of PsyDom::Start().
			CellClass *pTarget = MapClass::Instance->GetCellAt(this->Coords);
			CoordStruct coords = pTarget->GetCoords();
			coords.Z += pData->Dominator_FirstAnimHeight;

			AnimClass* pAnim = nullptr;
			if(AnimTypeClass* pAnimType = pData->Dominator_FirstAnim.Get(RulesClass::Instance->DominatorFirstAnim)) {
				GAME_ALLOC(AnimClass, pAnim, pAnimType, &coords);
			}
			PsyDom::Anim = pAnim;
		
			if(pData->SW_ActivationSound != -1) {
				VocClass::PlayAt(pData->SW_ActivationSound, &coords, nullptr);
			}

			pData->PrintMessage(pData->Message_Activate, this->Super->Owner);
			
			PsyDom::Status = PsychicDominatorStatus::Fire;

			// most likely LightUpdateTimer
			ScenarioClass::Instance->AmbientTimer.Start(1);
			ScenarioClass::UpdateLighting();

			return;
		}
	case PsychicDominatorStatus::Fire:
		{
			// wait for some percentage of the first anim to be
			// played until we strike.
			AnimClass* pAnim = PsyDom::Anim;
			if(pAnim) {
				int currentFrame = pAnim->Animation.Value;
				short frameCount = pAnim->Type->GetImage()->Frames;
				if(frameCount * pData->Dominator_FireAtPercentage * 0.01 > currentFrame) {
					return;
				}
			}

			PsyDom::Fire();

			PsyDom::Status = PsychicDominatorStatus::SecondAnim;
			return;
		}
	case PsychicDominatorStatus::SecondAnim:
		{
			// wait for the second animation to finish. (there may be up to
			// 10 frames still to be played.)
			AnimClass* pAnim = PsyDom::Anim;
			if(pAnim) {
				int currentFrame = pAnim->Animation.Value;
				short frameCount = pAnim->Type->GetImage()->Frames;

				if(frameCount - currentFrame > 10) {
					return;
				}
			}

			PsyDom::Status = PsychicDominatorStatus::Reset;
			return;
		}
	case PsychicDominatorStatus::Reset:
		{
			// wait for the last frame... WTF? 
			AnimClass* pAnim = PsyDom::Anim;
			if(pAnim) {
				int currentFrame = pAnim->Animation.Value;
				short frameCount = pAnim->Type->GetImage()->Frames;

				if(frameCount - currentFrame > 1) {
					return;
				}
			}

			PsyDom::Status = PsychicDominatorStatus::Over;

			PsyDom::Coords = CellStruct::Empty;
			PsyDom::Anim = nullptr;
			ScenarioClass::UpdateLighting();

			return;
		}
	case PsychicDominatorStatus::Over:
		{
			// wait for the light to go away.
			if(ScenarioClass::Instance->AmbientCurrent != ScenarioClass::Instance->AmbientTarget) {
				return;
			}

			// clean up
			SW_PsychicDominator::CurrentPsyDom = nullptr;
			PsyDom::Status = PsychicDominatorStatus::Inactive;
			ScenarioClass::UpdateLighting();
			this->Clock.TimeLeft = 0;
		}
	}
}
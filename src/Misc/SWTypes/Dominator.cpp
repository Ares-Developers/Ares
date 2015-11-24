#include "Dominator.h"
#include "../../Ares.h"
#include "../../Ext/Infantry/Body.h"
#include "../../Ext/WarheadType/Body.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

SuperClass* SW_PsychicDominator::CurrentPsyDom = nullptr;

bool SW_PsychicDominator::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicDominator);
}

SuperWeaponFlags SW_PsychicDominator::Flags() const
{
	return SuperWeaponFlags::NoEvent;
}

WarheadTypeClass* SW_PsychicDominator::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->DominatorWarhead);
}

int SW_PsychicDominator::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->DominatorDamage);
}

SWRange SW_PsychicDominator::GetRange(const SWTypeExt::ExtData* pData) const
{
	if(pData->SW_Range.empty()) {
		return SWRange(RulesClass::Instance->DominatorCaptureRange);
	}
	return pData->SW_Range;
}

void SW_PsychicDominator::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to PsychicDominator values
	pData->Dominator_FirstAnimHeight = 750;
	pData->Dominator_SecondAnimHeight = 0;
	pData->Dominator_Ripple = true;
	pData->Dominator_Capture = true;
	pData->Dominator_CaptureMindControlled = true;
	pData->Dominator_CapturePermaMindControlled = true;
	pData->Dominator_CaptureImmuneToPsionics = false;
	pData->Dominator_PermanentCapture = true;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_PsychicDominatorDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_PsychicDominatorReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_PsychicDominatorActivated");

	pData->Message_Abort = "Msg:DominatorActive";

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::PsychicDominator;
	pData->SW_AffectsTarget = SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::PsychicDominator);
}

void SW_PsychicDominator::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->Dominator_FirstAnimHeight.Read(exINI, section, "Dominator.FirstAnimHeight");
	pData->Dominator_SecondAnimHeight.Read(exINI, section, "Dominator.SecondAnimHeight");
	pData->Dominator_FirstAnim.Read(exINI, section, "Dominator.FirstAnim");
	pData->Dominator_SecondAnim.Read(exINI, section, "Dominator.SecondAnim");
	pData->Dominator_ControlAnim.Read(exINI, section, "Dominator.ControlAnim");
	pData->Dominator_FireAtPercentage.Read(exINI, section, "Dominator.FireAtPercentage");
	pData->Dominator_Capture.Read(exINI, section, "Dominator.Capture");
	pData->Dominator_Ripple.Read(exINI, section, "Dominator.Ripple");
	pData->Dominator_CaptureMindControlled.Read(exINI, section, "Dominator.CaptureMindControlled");
	pData->Dominator_CapturePermaMindControlled.Read(exINI, section, "Dominator.CapturePermaMindControlled");
	pData->Dominator_CaptureImmuneToPsionics.Read(exINI, section, "Dominator.CaptureImmuneToPsionics");
	pData->Dominator_PermanentCapture.Read(exINI, section, "Dominator.PermanentCapture");
}

bool SW_PsychicDominator::AbortFire(SuperClass* pSW, bool IsPlayer)
{
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

bool SW_PsychicDominator::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	if(pThis->IsCharged) {
		// we do not use PsyDom::Start() here. instead, we set a global state and
		// let the state machine take care of everything.
		SW_PsychicDominator::CurrentPsyDom = pThis;
		this->newStateMachine(Coords, pThis);
	}

	return true;
}

void PsychicDominatorStateMachine::Update()
{
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
				pAnim = GameCreate<AnimClass>(pAnimType, coords);
			}
			PsyDom::Anim = pAnim;
		
			auto sound = pData->SW_ActivationSound.Get(RulesClass::Instance->PsychicDominatorActivateSound);
			if(sound != -1) {
				VocClass::PlayAt(sound, coords, nullptr);
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
				int percentage = pData->Dominator_FireAtPercentage.Get(RulesClass::Instance->DominatorFireAtPercentage);
				if(frameCount * percentage / 100 > currentFrame) {
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

bool PsychicDominatorStateMachine::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(this->Deferment, RegisterForChange)
		.Success();
}

bool PsychicDominatorStateMachine::Save(AresStreamWriter &Stm) const {
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(this->Deferment)
		.Success();
}

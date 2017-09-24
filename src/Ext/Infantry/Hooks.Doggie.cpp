#include "Body.h"

#include <CellClass.h>

const int DoggiePanicMax = 300;

DEFINE_HOOK(51F628, InfantryClass_Guard_Doggie, 5)
{
	GET(InfantryClass*, pThis, ESI);
	GET(int, res, EAX);

	if(res != -1) {
		return 0x51F634;
	}

	// doggie sit down on tiberium handling
	if(pThis->Type->Doggie && !pThis->Crawling && !pThis->Target && !pThis->Destination) {
		if(!pThis->Facing.in_motion() && pThis->GetCell()->LandType == LandType::Tiberium) {
			if(pThis->Facing.current().value8() == Direction::East) {
				// correct facing, sit down
				pThis->PlayAnim(Sequence::Down);
			} else {
				// turn to correct facing
				DirStruct dir(3, Direction::East);
				pThis->Locomotor->Do_Turn(dir);
			}
		}
	}

	return 0x51F62D;
}

DEFINE_HOOK(518CB3, InfantryClass_ReceiveDamage_Doggie, 6)
{
	GET(InfantryClass*, pThis, ESI);

	// hurt doggie gets more panic
	if(pThis->Type->Doggie && pThis->IsRedHP()) {
		R->EDI(DoggiePanicMax);
	}

	return 0;
}

DEFINE_HOOK(51ABD7, InfantryClass_SetDestination_Doggie, 6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(AbstractClass*, pTarget, EBX);

	// doggie cannot crawl; has to stand up and run
	bool doggieStandUp = pTarget && pThis->Crawling && pThis->Type->Doggie;

	return doggieStandUp ? 0x51AC16 : 0;
}

DEFINE_HOOK(5200C1, InfantryClass_UpdatePanic_Doggie, 6)
{
	GET(InfantryClass*, pThis, ESI);
	auto pType = pThis->Type;

	if(!pType->Doggie) {
		return 0;
	}

	// if panicking badly, lay down on tiberium
	if(pThis->PanicDurationLeft >= DoggiePanicMax) {
		if(!pThis->Destination && !pThis->Locomotor->Is_Moving()) {
			if(pThis->GetCell()->LandType == LandType::Tiberium) {
				// is on tiberium. just lay down
				pThis->PlayAnim(Sequence::Down);
			} else {
				// search tiberium and abort current mission
				pThis->MoveToTiberium(16, false);
				if(pThis->Destination) {
					pThis->SetTarget(nullptr);
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}
			}
		}
	}

	if(!pType->Fearless) {
		--pThis->PanicDurationLeft;
	}

	return 0x52025A;
}

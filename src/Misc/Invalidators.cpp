#include <CCINIClass.h>
#include <TechnoTypeClass.h>
#include <WeaponTypeClass.h>
#include <AnimClass.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>
#include "Debug.h"
#include "../Ext/Rules/Body.h"

DEFINE_HOOK(477007, INIClass_GetSpeedType, 8)
{
	if(R->EAX() == -1) {
		GET_STACK(const char *, Section, 0x8C);
		LEA_STACK(const char *, Value, 0x8);
		GET_STACK(DWORD, caller, 0x88);
		/*
			this func is called from TechnoTypeClass::LoadFromINI and UnitTypeClass::LoadFromINI
			UnitTypeClass::CTOR initializes SpeedType to -1
			UnitTypeClass::LoadFromINI overrides it to (this->Crusher ? Track : Wheel) just before reading its SpeedType
			so we should not alert if we're responding to a TType read and our subject is a UnitType, or all VehicleTypes without an explicit ST declaration will get dinged
		*/
		if(strlen(Value)) {
			if(caller != 0x7121E5 || R->EBP<TechnoTypeClass *>()->WhatAmI() != abs_UnitType) {
				Debug::INIParseFailed(Section, "SpeedType", Value);
			}
		}
	}
	return 0;
}

DEFINE_HOOK(474E8E, INIClass_GetMovementZone, 5)
{
	if(R->EAX() == -1) {
		GET_STACK(const char *, Section, 0x2C);
		LEA_STACK(const char *, Value, 0x8);
		if(strlen(Value)) {
			Debug::INIParseFailed(Section, "MovementZone", Value);
		}
	}
	return 0;
}

DEFINE_HOOK(47542A, INIClass_GetArmorType, 6)
{
	if(R->EAX() == -1) {
		GET_STACK(const char *, Section, 0x8C);
		LEA_STACK(const char *, Value, 0x8);
		if(strlen(Value)) {
			Debug::INIParseFailed(Section, "Armor", Value);
		}
	}
	return 0;
}

DEFINE_HOOK(474DEE, INIClass_GetFoundation, 7)
{
	if(R->EAX() == -1) {
		GET_STACK(const char *, Section, 0x2C);
		LEA_STACK(const char *, Value, 0x8);
		if(_strcmpi(Value, "Custom")) {
			Debug::INIParseFailed(Section, "Foundation", Value);
		}
	}
	return 0;
}

DEFINE_HOOK(687C16, INIClass_ReadScenario_ValidateThings, 6)
{
	/*
		all the INI files have been read
		add whatever code you need to validate that objects are in a valid state
		(e.g., to make sure crevio hasn't stuck a MovementZone=Retarded on anything)
		to reduce chances of crashing later
	*/
	for(int i = 0; i < TechnoTypeClass::Array->Count; ++i) {
		TechnoTypeClass *Item = reinterpret_cast<TechnoTypeClass *>(TechnoTypeClass::Array->Items[i]);

		bool IsFoot = Item->WhatAmI() != abs_BuildingType;

		if(IsFoot && Item->SpeedType == -1) {
			Debug::DevLog(Debug::Error, "[%s]SpeedType is invalid!\n", Item->ID);
		}

		if(IsFoot && Item->MovementZone == -1) {
			Debug::DevLog(Debug::Error, "[%s]MovementZone is invalid!\n", Item->ID);
		}

		if(Item->Armor == -1) {
			Debug::DevLog(Debug::Error, "[%s]Armor is invalid!\n", Item->ID);
		}

		if(Item->Passengers > 0 && Item->SizeLimit < 1) {
			Debug::DevLog(Debug::Error, "[%s]Passengers=%d and SizeLimit=%d!\n", Item->ID, Item->Passengers, Item->SizeLimit);
		}
	}

	for(int i = 0; i < WeaponTypeClass::Array->Count; ++i) {
		WeaponTypeClass *Item = WeaponTypeClass::Array->Items[i];
		if(!Item->Warhead) {
			Debug::DevLog(Debug::Error, "Weapon[%s] has no Warhead! This usually indicates one of two things:\n"
				"- The weapon was created too late and its rules weren't read (see WEEDGUY hack);\n"
				"- The weapon's name was misspelled.\n"
			, Item->get_ID());
		}
	}

	if(Ares::bStrictParser && Debug::bParserErrorDetected) {
		Debug::FatalErrorAndExit("One or more errors were detected while parsing the INI files.\r\n"
				"Please review the contents of the debug log and correct them.");
	}

	// #1000
	if(RulesExt::ExtData *AresGeneral = RulesExt::Global()) {
		if(!!AresGeneral->CanMakeStuffUp) {
			if(RulesClass* StockGeneral = RulesClass::Global()) { // well, the modder *said* we can make stuff up, so...
				Randomizer *r = &ScenarioClass::Instance->Random;

				StockGeneral->VeteranRatio = r->RandomRanged(1, 500) / 100.0;
				StockGeneral->BuildSpeed = r->RandomRanged(1, 350) / 100.0;
				StockGeneral->BuildupTime = r->RandomRanged(1, 50) / 100.0;
				StockGeneral->RefundPercent = r->RandomRanged(1, 900) / 100.0;
				StockGeneral->GrowthRate /= r->RandomRanged(1, 5);
				//StockGeneral->GameSpeedBias += r->RandomRanged(-40, 40) / 100.0;
				StockGeneral->Stray = r->RandomRanged(1, 5);
				StockGeneral->FlightLevel = r->RandomRanged(900, 2500);

				if(r->RandomRanged(1, 10) == 3) {
					StockGeneral->ParachuteMaxFallRate *= -1;
					StockGeneral->NoParachuteMaxFallRate -= 5;
				}

				// for extra WTF-ness:
				int monkey = InfantryTypeClass::FindIndex("JOSH");
				int camel = InfantryTypeClass::FindIndex("CAML");
				int cow = InfantryTypeClass::FindIndex("COW");
				bool zooTime = r->RandomRanged(1, 5) == 3;
				if((monkey != -1) && zooTime) {
					StockGeneral->AlliedCrew = InfantryTypeClass::Array->GetItem(monkey);
				}
				zooTime = r->RandomRanged(1, 5) == 3;
				if((camel != -1) && zooTime) {
					StockGeneral->SovietCrew = InfantryTypeClass::Array->GetItem(camel);
				}
				zooTime = r->RandomRanged(1, 5) == 3;
				if((cow != -1) && zooTime) {
					StockGeneral->ThirdCrew = InfantryTypeClass::Array->GetItem(cow);
				}
				//-

				StockGeneral->HoverHeight += r->RandomRanged(-30, 30);
				StockGeneral->WindDirection = r->RandomRanged(0, 7);
				StockGeneral->MaximumQueuedObjects += r->RandomRanged(-5, 5);
				StockGeneral->MaxWaypointPathLength += r->RandomRanged(-5, 5);
				StockGeneral->CruiseHeight += r->RandomRanged(-200, 200);

				auto getRandomColor = [r](int curCol) -> int {
					// assuming the default range of 0-13
					switch(curCol) {
						case 0: return curCol + r->RandomRanged(0, 1);
						case 13: return curCol + r->RandomRanged(-1, 0);
						default:
							if((curCol > 0) && (curCol < 13)) {
								return curCol + r->RandomRanged(-1, 1);
							} else {
								return curCol;
							}
					}
				};

				StockGeneral->LaserTargetColor = getRandomColor(StockGeneral->LaserTargetColor);
				StockGeneral->IronCurtainColor = getRandomColor(StockGeneral->IronCurtainColor);
				StockGeneral->BerserkColor = getRandomColor(StockGeneral->BerserkColor);
				StockGeneral->ForceShieldColor = getRandomColor(StockGeneral->ForceShieldColor);

				StockGeneral->PoseDir = r->RandomRanged(0, 255);
				StockGeneral->DeployDir = r->RandomRanged(0, 255);

				StockGeneral->Gravity += r->RandomRanged(-2, 2);
				StockGeneral->IvanTimedDelay += r->RandomRanged(-150, 150);

				StockGeneral->PlayerAutoCrush = r->RandomRanged(1, 3) == 3;
				StockGeneral->PlayerReturnFire = r->RandomRanged(1, 3) == 3;
				StockGeneral->PlayerScatter = r->RandomRanged(1, 3) == 3;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(55AFB3, LogicClass_Update_1000, 6)
{

	if(RulesExt::ExtData *AresGeneral = RulesExt::Global()) {
		if(!!AresGeneral->CanMakeStuffUp) {
			if(Unsorted::CurrentFrame % 90 == 0) {

				auto RandomRanged = [](int Min, int Max) {
					return ScenarioClass::Instance->Random.RandomRanged(Min, Max);
				};

				for(int i = 0; i < InfantryClass::Array->Count; ++i) {
					auto Inf = InfantryClass::Array->GetItem(i);
					if(Inf->IsFallingDown) {
						if(auto paraAnim = Inf->Parachute) {
							int limit = RandomRanged(-300, 300) + 3000;
							if(Inf->GetHeight() >= limit) {
								paraAnim->RemainingIterations = 0;
								Inf->HasParachute = false;
								Inf->FallRate = -1;
								Inf->IsABomb = true; // ffffuuuu....
							}
						}

						continue;
					}

					if(Inf->Fetch_ID() % 500 == Unsorted::CurrentFrame % 500) { // I have no idea how often this happens, btw
						if(RandomRanged(1, 100) <= 2) {
							Inf->Panic();
						} else if(RandomRanged(1, 100) >= 98) {
							Inf->UnPanic();
						}
					}
				}
			}
		}
	}

	return 0;
}


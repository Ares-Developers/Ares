#include <CCINIClass.h>
#include <TechnoTypeClass.h>
#include <WeaponTypeClass.h>
#include <AnimClass.h>
#include <InfantryClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include "Debug.h"
#include "EMPulse.h"
#include "../Ext/Rules/Body.h"
#include "../Ext/HouseType/Body.h"
#include "../Ext/Side/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/BuildingType/Body.h"
#include "../Utilities/Helpers.Alex.h"
#include <vector>
#include <algorithm>
#include <string>
#include "../Ares.version.h"

DEFINE_HOOK(477007, INIClass_GetSpeedType, 8)
{
	if(R->EAX() == -1) {
		GET_STACK(const char *, Section, 0x8C);
		GET_STACK(const char *, Key, 0x90);
		LEA_STACK(const char *, Value, 0x8);
		GET_STACK(DWORD, caller, 0x88);
		/*
			this func is called from TechnoTypeClass::LoadFromINI and UnitTypeClass::LoadFromINI
			UnitTypeClass::CTOR initializes SpeedType to -1
			UnitTypeClass::LoadFromINI overrides it to (this->Crusher ? Track : Wheel) just before reading its SpeedType
			so we should not alert if we're responding to a TType read and our subject is a UnitType, or all VehicleTypes without an explicit ST declaration will get dinged
		*/
		if(caller != 0x7121E5u
			|| !Helpers::Alex::is_any_of(R->EBP<TechnoTypeClass*>()->WhatAmI(),
				AbstractType::UnitType, AbstractType::BuildingType))
		{
			Debug::INIParseFailed(Section, Key, Value);
		}
	}
	return 0;
}

DEFINE_HOOK(474E8E, INIClass_GetMovementZone, 5)
{
	GET_STACK(const char *, Section, 0x2C);
	GET_STACK(const char *, Key, 0x30);
	LEA_STACK(const char *, Value, 0x8);
	Debug::INIParseFailed(Section, Key, Value);
	return 0;
}

DEFINE_HOOK(474DEE, INIClass_GetFoundation, 7)
{
	GET_STACK(const char *, Section, 0x2C);
	GET_STACK(const char *, Key, 0x30);
	LEA_STACK(const char *, Value, 0x8);
	if(_strcmpi(Value, "Custom")) {
		Debug::INIParseFailed(Section, Key, Value);
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

	// create an array of crew for faster lookup
	DynamicVectorClass<InfantryTypeClass*> Crews(SideClass::Array->Count, nullptr);
	for(auto const& pSide : *SideClass::Array) {
		Crews.AddItem(SideExt::ExtMap.Find(pSide)->GetCrew());
	}

	for(auto const& pItem : *TechnoTypeClass::Array) {
		auto const isFoot = pItem->WhatAmI() != AbstractType::BuildingType;

		if(isFoot && pItem->SpeedType == SpeedType::None) {
			Debug::Log(
				Debug::Severity::Error, "[%s]SpeedType is invalid!\n",
				pItem->ID);
			Debug::RegisterParserError();
		}

		if(isFoot && pItem->MovementZone == MovementZone::None) {
			Debug::Log(
				Debug::Severity::Error, "[%s]MovementZone is invalid!\n",
				pItem->ID);
			Debug::RegisterParserError();
		}

		// this should never fire, default is 0 (which is valid), not -1.
		// see hook at 4753F0. -AlexB
		//if(pItem->Armor == static_cast<Armor>(-1)) { 
		//	Debug::Log(
		//		Debug::Severity::Error, "[%s]Armor is invalid!\n", pItem->ID);
		//	Debug::RegisterParserError();
		//}

		if(pItem->Passengers > 0 && pItem->SizeLimit < 1) {
			Debug::Log(
				Debug::Severity::Error, "[%s]Passengers=%d and SizeLimit=%d!\n",
				pItem->ID, pItem->Passengers, pItem->SizeLimit);
			Debug::RegisterParserError();
		}

		auto const pExt = TechnoTypeExt::ExtMap.Find(pItem);
		if(pItem->PoweredUnit && !pExt->PoweredBy.empty()) {
			Debug::Log(
				Debug::Severity::Error,
				"[%s] uses both PoweredUnit=yes and PoweredBy=!\n", pItem->ID);
			Debug::RegisterParserError();
			pItem->PoweredUnit = false;
		}
		if(auto const pPowersUnit = pItem->PowersUnit) {
			auto const pExtraData = TechnoTypeExt::ExtMap.Find(pPowersUnit);
			if(!pExtraData->PoweredBy.empty()) {
				Debug::Log(
					Debug::Severity::Error,
					"[%s]PowersUnit=%s, but [%s] uses PoweredBy=!\n",
					pItem->ID, pPowersUnit->ID, pPowersUnit->ID);
				Debug::RegisterParserError();
				pItem->PowersUnit = nullptr;
			}
		}

		// if empty, set survivor pilots to the corresponding side's Crew
		{
			auto const count = Math::min(
				pExt->Survivors_Pilots.Count, Crews.Count);
			for(auto j = 0; j < count; ++j) {
				if(!pExt->Survivors_Pilots[j]) {
					pExt->Survivors_Pilots[j] = Crews[j];
				}
			}
		}

		for(int k = static_cast<int>(pExt->ClonedAt.size()) - 1; k >= 0; --k) {
			auto const pCloner = pExt->ClonedAt[k];
			if(pCloner->Factory != AbstractType::None) {
				pExt->ClonedAt.erase(pExt->ClonedAt.begin() + k);
				Debug::Log(Debug::Severity::Error,
					"[%s]ClonedAt includes %s, but %s has Factory= settings. "
					"This combination is not supported.\n(Protip: Factory= is "
					"not what controls unit exit behaviour, WeaponsFactory= "
					"and GDI/Nod/YuriBarracks= is.)\n", pItem->ID, pCloner->ID,
					pCloner->ID);
				Debug::RegisterParserError();
			}
		}

		if(!isFoot) {
			auto const pBItem = abstract_cast<BuildingTypeClass*>(pItem);
			auto const pBExt = BuildingTypeExt::ExtMap.Find(pBItem);
			if(pBExt->CloningFacility && pBItem->Factory != AbstractType::None) {
				pBExt->CloningFacility = false;
				Debug::Log(Debug::Severity::Error,
					"[%s] cannot have both CloningFacility= and Factory=.\n",
					pItem->ID);
				Debug::RegisterParserError();
			}
		}
	}

	for(auto const pBType : *BuildingTypeClass::Array) {
		auto const techLevel = pBType->TechLevel;
		if(techLevel < 0 || techLevel > RulesClass::Instance->TechLevel) {
			continue;
		}
		if(pBType->BuildCat == BuildCat::DontCare) {
			pBType->BuildCat = ((pBType->SuperWeapon != -1) || pBType->IsBaseDefense || pBType->Wall)
				? BuildCat::Combat : BuildCat::Infrastructure;
			auto const catName = (pBType->BuildCat == BuildCat::Combat)
				? "Combat" : "Infrastructure";
			Debug::Log(Debug::Severity::Warning,
				"Building Type [%s] does not have a valid BuildCat set!\n"
				"It was reset to %s, but you should really specify it "
				"explicitly.\n", pBType->ID, catName);
			Debug::RegisterParserError();
		}
	}

	for(auto const& pItem : *WeaponTypeClass::Array) {
		constexpr auto const Msg =
			"Weapon[%s] has no %s! This usually indicates one of two things:\n"
			"- The weapon was created too late and its rules weren't read "
			"(see WEEDGUY hack);\n- The weapon's name was misspelled.\n";

		if(!pItem->Warhead) {
			Debug::Log(Debug::Severity::Error, Msg, pItem->ID, "Warhead");
			Debug::RegisterParserError();
		}

		if(!pItem->Projectile) {
			Debug::Log(Debug::Severity::Error, Msg, pItem->ID, "Projectile");
			Debug::RegisterParserError();
		}
	}

	{ // new scope to keep it tidy
		std::pair<const char*, int> LimitedClasses[] = {
			{ "BuildingTypes", BuildingTypeClass::Array->Count },
			{ "VehicleTypes", UnitTypeClass::Array->Count },
			{ "InfantryTypes", InfantryTypeClass::Array->Count },
			{ "AircraftTypes", AircraftTypeClass::Array->Count }
		};

		for(auto const& limited : LimitedClasses) {
			if(limited.second > 512) {
				Debug::Log(Debug::Severity::Warning,
					"The [%s] list contains more than 512 entries. This might "
					"result in unexpected behaviour and crashes.\n",
					limited.first);
			}
		}
	}

	for(auto const& pConst : RulesClass::Instance->BuildConst) {
		if(!pConst->AIBuildThis) {
			Debug::Log(Debug::Severity::Warning,
				"[AI]BuildConst= includes [%s], which doesn't have "
				"AIBuildThis=yes!\n", pConst->ID);
		}
	}

	if(OverlayTypeClass::Array->Count > 255) {
		Debug::Log(Debug::Severity::Error,
			"Only 255 OverlayTypes are supported.\n");
		Debug::RegisterParserError();
	}

	if(Ares::bStrictParser && Debug::bParserErrorDetected) {
		Debug::FatalErrorAndExit(
			"One or more errors were detected while parsing the INI files.\r\n"
			"Please review the contents of the debug log and correct them.");
	}

	// #1000
	if(auto const AresGeneral = RulesExt::Global()) {
		if(AresGeneral->CanMakeStuffUp) {
			Randomizer *r = &ScenarioClass::Instance->Random;
			if(RulesClass* StockGeneral = RulesClass::Global()) { // well, the modder *said* we can make stuff up, so...

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

			// since the modder is..."faithful" enough to believe CanMakeStuffUp works, he won't need logs anyway
			Debug::LogFileClose(0x1D107);
			Debug::LogFileOpen(); // this annihilates the previous log contents

			Debug::Log("Initialized Ares version: %s\n"
					"Checking available disk space\n"
					"Using GetDiskFreeSpaceEx\n"
					"Free disk space is 3235830701 bytes\n"
					"Init Encryption Keys.\n"
					"Init_Keys - declarations\n"
					"Init_Keys - Load\n"
					"Init_Keys - Init fast key\n"
					"EXPANDMD99.MIX EXPANDMD98.MIX EXPANDMD97.MIX EXPANDMD06.MIX EXPANDMD01.MIX CACHE.MIX CACHE.MIX CACHE.MIX LOCAL.MIXMaxLabelLen = 31\n"
					"Language: Elbonian\n"
					"Focus_Restore()\n"
					"Focus_Restore(): _MouseCaptured = false\n"
					"Focus gained\n"
					"Defeat\n"
					"Defeat\n"
					"Prep direct draw.\n"
					"Prep direct draw.\n"
					"SetDisplayMode: 4200x690x48\n"
					"Checking hardware region fill capability...OK\n"
					"Checking overlapped blit capability...OK\n"
					"Display mode set\n"
					"DSurface::Create_Primary\n"
					"DSurface::AllowStretchBlits = true\n"
					"DSurface::AllowHWFill = true\n"
					"DSurface::Create_Primary - Creating surface\n"
					"CreateSurface OK\n"
					"DSurface::Create_Primary done\n"
					"Calc_Confining_Rect(0,0,800,600)\n"
					"Profile: CPU:1 (1998Mhz Pentium Pro)\n"
					"Profile: RAM:9 (256Mb)\n"
					"Profile: VRAM:10 (1293Mb)\n"
					"Profile: VRAM speed:55 (446 blits per second)\n"
					"Overall performance profile = 1\n"
					"Main_Game\n"
					"Init Game\n",

					VERSION_STR);

			// we can't just leave him with no log, though...that'd be depressing and suspicious
			static const char* const listOfLines[] = {
				"Gremlins found in [General], initializing microwave algorithms",
				"Finding and removing porn\nFound 4269 files\nDeleting blondes\nDeleting brunettes\nDeleting redheads\nDeleting shemales\nDeleting midgets\nDeleting horses",
				"Found pirated music, deleting 2342 tracks",
				"Analyzing unit parameters\nMod's balance is crappy",
				"Cannot initialize sound - device occupied by crappy music",
				"MIX loading aborted; parser busy looking at Tanya porn",
				"Checking player's hardware\nPlayer's hardware is embarrassingly small",
				"Loading SHP parser\nSHP parser says the graphics of this mod are fugly",
				"Reversing polarity",
				"Questioning the purpose of life",
				"To blit, or not to blit- that is the question:\nWhether 'tis nobler in the mind to suffer\nThe slings and arrows of pathetic modding,\nOr to take arms against a sea of troubles\nAnd, by opposing, end them.",
				"You look nice today, do you have a new haircut?",
				"Initializing SkyNet protocols",
				"Scanning WLANs\nObtaining WPA2 pre-shared key\nDownloading horse porn",
				"Checking \"hardware region\" fill capability...OK",
				"Checking overlapped tit capability...OK",
				"VisibleRectum: 800x600",
				"Your toaster is on fire",
				"Having sex with your dog",
				"WincockInterface constructed\nWincockInterface init.\nAbout to call WSAStartup\nChanged my mind, waiting for him to call\nWincock initialised OK anyway\nWincock version is 1.1 inches",
				"Parsing [AudioVisual]\nTurning away in disgust",
				"Shaking my head at the modder",
				"Staring wearily into the distance",
				"Wondering when this will finally end",
				"How long can a rules.ini be, really?",
				"On this file handle, show me exactly where the modder touched you, please",
				"Mowing player's lawn",
				"Yawning about the boredom of this mod",
				"Randomly flipping bits on the map",
				"Initializing Internal Error countdown",
				"Init random number\nRolling dice\nSeed is 6",
				"Generating random number\nRandom number is 4",
				"Generating random number\nRandom number is 4",
				"Generating random number\nRandom number is 4",
				"Generating random number\nRandom number is 4",
				"Generating random number\nRandom number is 4",
				"Taping Tanya making out with Boris",
				"Mocking the mod's SHPs",
				"Repainting voxels\nVoxels are now pink",
				"Questioning modder's sexual orientation",
				"Poking and prodding the engine until it finally gets up",
				"Creating TacticalMap\nTactic is: Build APOCs until you run out of money, rush the opponent",
				"RadarClass::First_Time()\nRadarClass came too fast",
				"Init Commandments\nBuffer overflow: Not enough space for 10 Commandments",
				"Getting file from host\nFondling host",
				"Calling Get_File_From_Host to receive the file download\nGetting file from host\nRequesting file download\nSending global ack packet to port 0\nHost responded with file info\nFile name is two_chicks_blowing_a_horse_and_swallowing.bin\nSending file info received ack\nReceiving download of file two_chicks_blowing_a_horse_and_swallowing.bin",
				"Activating webcam\nRecording player\nPutting video on YouTube",
				"Activating webcam\nRecording player\nPutting video on YouPorn",
				"Watching paint dry",
				"Executing Order 66",
				"Debugging debugger",
				"Poking art.ini",
				"Touching sound.ini with a 10-foot-pole"
			};

			const int listSize = std::size(listOfLines); // get item count of listOfLines

			std::vector<const char*> lines(listOfLines, listOfLines + listSize); // modifiable list of lines

			for(int i = 0; i < 10; ++i) {
				// 'i' used lines have been shuffled to the end already
				auto const available = static_cast<int>(lines.size()) - i;
				auto const index = r->RandomRanged(0, available - 1);

				// mark this line as used by swapping it with last
				auto const last = lines.begin() + available - 1;
				std::iter_swap(lines.begin() + index, last);

				// print the line
				Debug::Log("%s\n", *last);
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

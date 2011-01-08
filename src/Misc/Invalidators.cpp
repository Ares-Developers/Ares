#include <CCINIClass.h>
#include <TechnoTypeClass.h>
#include <WeaponTypeClass.h>
#include <AnimClass.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>
#include "Debug.h"
#include "../Ext/Rules/Body.h"
#include <vector>
#include <algorithm>
#include "../Ares.version.h"

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
			Debug::LogFileRemove();
			Debug::MakeLogFile();
			Debug::LogFileOpen();

			char defaultStuff[1050];

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
					"Init Game",

					VERSION_STR);

			// we can't just leave him with no log, though...that'd be depressing and suspicious
			auto getRandomLogLine = [r]() -> char * {
				static std::vector<char *> listOfLines;
				if(listOfLines.empty()) { // if the list wasn't populated yet, populate it
					listOfLines.push_back("Gremlins found in [General], initializing microwave algorithms");
					listOfLines.push_back("Finding and removing porn\n\tFound 4269 files\n\tDeleting blondes\n\tDeleting brunettes\n\tDeleting redheads\n\tDeleting shemales\n\tDeleting midgets\n\tDeleting horses");
					listOfLines.push_back("Found pirated music, deleting 2342 tracks");
					listOfLines.push_back("Analyzing unit parameters\nMod's balance is crappy");
					listOfLines.push_back("Cannot initialize sound - device occupied by crappy music");
					listOfLines.push_back("MIX loading aborted; parser busy looking at Tanya porn");
					listOfLines.push_back("Checking player's hardware\n\tPlayer's hardware is embarrassingly small");
					listOfLines.push_back("Loading SHP parser\nSHP parser says the graphics of this mod are fugly");
					listOfLines.push_back("Reversing polarity");
					listOfLines.push_back("Questioning the purpose of life");
					listOfLines.push_back("To blit, or not to blit- that is the question:\nWhether 'tis nobler in the mind to suffer\nThe slings and arrows of pathetic modding,\nOr to take arms against a sea of troubles\nAnd, by opposing, end them.");
					listOfLines.push_back("You look nice today, do you have a new haircut?");
					listOfLines.push_back("Initializing SkyNet protocols");
					listOfLines.push_back("Scanning WLANs\nObtaining WPA2 pre-shared key\nDownloading horse porn");
					listOfLines.push_back("Checking \"hardware region\" fill capability...OK");
					listOfLines.push_back("Checking overlapped tit capability...OK");
					listOfLines.push_back("VisibleRectum: 800x600");
					listOfLines.push_back("Your toaster is on fire");
					listOfLines.push_back("Having sex with your dog");
					listOfLines.push_back("WincockInterface constructed\nWincockInterface init.\nAbout to call WSAStartup\nChanged my mind, waiting for him to call\nWincock initialised OK anyway\nWincock version is 1.1 inches");
					listOfLines.push_back("Parsing [AudioVisual]\nTurning away in disgust");
					listOfLines.push_back("Shaking my head at the modder");
					listOfLines.push_back("Staring wearily into the distance");
					listOfLines.push_back("Wondering when this will finally end");
					listOfLines.push_back("How long can a rules.ini be, really?");
					listOfLines.push_back("On this file handle, show me exactly where the modder touched you, please");
					listOfLines.push_back("Mowing player's lawn");
					listOfLines.push_back("Yawning about the boredom of this mod");
					listOfLines.push_back("Randomly flipping bits on the map");
					listOfLines.push_back("Initializing Internal Error countdown");
					listOfLines.push_back("Init random number\nRolling dice\nSeed is 6");
					listOfLines.push_back("Generating random number\nRandom number is 4");
					listOfLines.push_back("Generating random number\nRandom number is 4");
					listOfLines.push_back("Generating random number\nRandom number is 4");
					listOfLines.push_back("Generating random number\nRandom number is 4");
					listOfLines.push_back("Generating random number\nRandom number is 4");
					listOfLines.push_back("Taping Tanya making out with Boris");
					listOfLines.push_back("Mocking the mod's SHPs");
					listOfLines.push_back("Repainting voxels\nVoxels are now pink");
					listOfLines.push_back("Questioning modder's sexual orientation");
					listOfLines.push_back("Poking and prodding the engine until it finally gets up");
					listOfLines.push_back("Creating TacticalMap\nTactic is: Build APOCs until you run out of money, rush the opponent");
					listOfLines.push_back("RadarClass::First_Time()\nRadarClass came too fast");
					listOfLines.push_back("Init Commandments\nBuffer overflow: Not enough space for 10 Commandments");
					listOfLines.push_back("Getting file from host\nFondling host");
					listOfLines.push_back("Calling Get_File_From_Host to receive the file download\nGetting file from host\nRequesting file download\nSending global ack packet to port 0\nHost responded with file info\nFile name is two_chicks_blowing_a_horse_and_swallowing.bin\nSending file info received ack\nReceiving download of file two_chicks_blowing_a_horse_and_swallowing.bin");
					listOfLines.push_back("Activating webcam\nRecording player\nPutting video on YouTube");
					listOfLines.push_back("Activating webcam\nRecording player\nPutting video on YouPorn");
					listOfLines.push_back("Watching paint dry");
					listOfLines.push_back("Executing Order 66");
					listOfLines.push_back("Debugging debugger");
					listOfLines.push_back("Poking art.ini");
					listOfLines.push_back("Touching sound.ini with a 10-foot-pole");
				}


				if(!listOfLines.empty()) {
					static std::vector<int> usedLines; // list of lines already used in this log
					int logLineNo = r->RandomRanged(0, listOfLines.size()-1);
					std::vector<int>::iterator result;

					result = find(usedLines.begin(), usedLines.end(), logLineNo); // check if this line was used before
					while(result != usedLines.end()) { // if so, find a new one
						logLineNo = r->RandomRanged(0, listOfLines.size()-1);
						result = find(usedLines.begin(), usedLines.end(), logLineNo);
					}

					// mark this line as used
					usedLines.push_back(logLineNo);

					// return the line
					return listOfLines.at(logLineNo);
				} else {
					return "List of Log Lines not populated.";
				}
			};

			for(int i = 0; i < 10; ++i) {
				Debug::Log("%s\n", getRandomLogLine());
			}

			// closing and reopening once to separate "creative" log parts from future log parts
			Debug::LogFileClose(0x1D107);
			Debug::LogFileOpen();
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


#ifndef ENUMS_H_
#define ENUMS_H_

#include "./../Ares.h"

class SuperWeaponAITargetingMode {
public:
	typedef unsigned char Value;
	enum {
		None = 0x0,
		Nuke = 0x1,
		LightningStorm = 0x2,
		PsychicDominator = 0x3,
		ParaDrop = 0x4,
		GeneticMutator = 0x5,
		ForceShield = 0x6,
		NoTarget = 0x7,
		Offensive = 0x8,
		Stealth = 0x9,
		Self = 0xA,
		Base = 0xB
	};

	static bool Parse(char* key, Value* value) {
		if(key && value) {
			if(!_strcmpi(key, "none")) {
				*value = SuperWeaponAITargetingMode::None;
			} else if(!_strcmpi(key, "nuke")) {
				*value = SuperWeaponAITargetingMode::Nuke;
			} else if(!_strcmpi(key, "lightningstorm")) {
				*value = SuperWeaponAITargetingMode::LightningStorm;
			} else if(!_strcmpi(key, "psychicdominator")) {
				*value = SuperWeaponAITargetingMode::PsychicDominator;
			} else if(!_strcmpi(key, "paradrop")) {
				*value = SuperWeaponAITargetingMode::ParaDrop;
			} else if(!_strcmpi(key, "geneticmutator")) {
				*value = SuperWeaponAITargetingMode::GeneticMutator;
			} else if(!_strcmpi(key, "forceshield")) {
				*value = SuperWeaponAITargetingMode::ForceShield;
			} else if(!_strcmpi(key, "notarget")) {
				*value = SuperWeaponAITargetingMode::NoTarget;
			} else if(!_strcmpi(key, "offensive")) {
				*value = SuperWeaponAITargetingMode::Offensive;
			} else if(!_strcmpi(key, "stealth")) {
				*value = SuperWeaponAITargetingMode::Stealth;
			} else if(!_strcmpi(key, "base")) {
				*value = SuperWeaponAITargetingMode::Base;
			} else if(!_strcmpi(key, "self")) {
				*value = SuperWeaponAITargetingMode::Self;
			} else {
				return false;
			}
			return true;
		}
		return false;
	}
};

class SuperWeaponTarget {
public:
	typedef unsigned char Value;
	enum {
		None = 0x0,
		Land = 0x1,
		Water = 0x2,
		NoContent = 0x4,
		Infantry = 0x8,
		Unit = 0x10,
		Building = 0x20,

		All = 0xFF,
		AllCells = Land | Water,
		AllTechnos = Infantry | Unit | Building,
		AllContents = NoContent | AllTechnos
	};

	static bool Parse(char* key, Value* value) {
		if(key && value) {
			Value ret = SuperWeaponTarget::None;
			for(char *cur = strtok(key, ","); cur; cur = strtok(NULL, Ares::readDelims)) {
				if(!_strcmpi(cur, "land")) {
					ret |= SuperWeaponTarget::Land;
				} else if(!_strcmpi(cur, "water")) {
					ret |= SuperWeaponTarget::Water;
				} else if(!_strcmpi(cur, "empty")) {
					ret |= SuperWeaponTarget::NoContent;
				} else if(!_strcmpi(cur, "infantry")) {
					ret |= SuperWeaponTarget::Infantry;
				} else if(!_strcmpi(cur, "units")) {
					ret |= SuperWeaponTarget::Unit;
				} else if(!_strcmpi(cur, "buildings")) {
					ret |= SuperWeaponTarget::Building;
				} else if(!_strcmpi(cur, "all")) {
					ret |= SuperWeaponTarget::All;
				}
			}
			*value = ret;
			return true;
		}
		return false;
	}
};

class SuperWeaponAffectedHouse {
public:
	typedef unsigned char Value;
	enum {
		None = 0x0,
		Owner = 0x1,
		Allies = 0x2,
		Team = 0x3,
		Enemies = 0x4,
		NotAllies = 0x5,
		NotOwner = 0x6,
		All = 0x7
	};

	static bool Parse(char* key, Value* value) {
		if(key && value) {
			Value ret = SuperWeaponAffectedHouse::None;
			for(char *cur = strtok(key, ","); cur; cur = strtok(NULL, ",")) {
				if(!_strcmpi(cur, "owner")) {
					ret |= SuperWeaponAffectedHouse::Owner;
				} else if(!_strcmpi(cur, "allies")) {
					ret |= SuperWeaponAffectedHouse::Allies;
				} else if(!_strcmpi(cur, "enemies")) {
					ret |= SuperWeaponAffectedHouse::Enemies;
				} else if(!_strcmpi(cur, "team")) {
					ret |= SuperWeaponAffectedHouse::Team;
				} else if(!_strcmpi(cur, "others")) {
					ret |= SuperWeaponAffectedHouse::NotOwner;
				} else if(!_strcmpi(cur, "all")) {
					ret |= SuperWeaponAffectedHouse::All;
				}
			}
			*value = ret;
			return true;
		}
		return false;
	}
};

class SuperWeaponFlags {
public:
	typedef unsigned short Value;
	enum {
		None = 0x0,
		NoAnim = 0x1,
		NoSound = 0x2,
		NoEvent = 0x4,
		NoEVA = 0x8,
		NoMoney = 0x10,
		NoCleanup = 0x20,
		NoMessage = 0x40,
		PreClick = 0x80,
		PostClick = 0x100
	};
};

#endif
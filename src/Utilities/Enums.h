#ifndef ENUMS_H_
#define ENUMS_H_

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
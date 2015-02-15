#pragma once

class TechnoClass;
class HouseClass;

class PoweredUnitClass
{
private:
	static const int ScanInterval = 15;		//!< Minimum delay between scans in frames.

	TechnoClass* Techno;
	int LastScan;							//!< Frame number when the last scan was performed.
	bool Powered;							//!< Whether the unit has a building providing power. NOT the same as being online.

	bool IsPoweredBy(HouseClass* Owner) const;
	void PowerUp();
	bool PowerDown();
public:
	PoweredUnitClass(TechnoClass* Techno) : Techno(Techno), LastScan(0), Powered(true)
	{ }

	~PoweredUnitClass() = default;

	//!< Updates this Powered Unit's status. Returns whether the unit should stay alive.
	bool Update();

	//!< Whether the unit has a building providing power. NOT the same as being online.
	inline bool IsPowered() const {
		return this->Powered;
	}

	bool Load(AresStreamReader &Stm, bool RegisterForChange);
	bool Save(AresStreamWriter &Stm) const;
};

template <>
struct Savegame::ObjectFactory<PoweredUnitClass> {
	std::unique_ptr<PoweredUnitClass> operator() (AresStreamReader &Stm) const {
		return std::make_unique<PoweredUnitClass>(nullptr);
	}
};

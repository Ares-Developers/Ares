#ifndef NEW_SW_TYPE_H
#define NEW_SW_TYPE_H

#include "../Ext/SWType/Body.h"
#include "../Utilities/Enums.h"

#include <vector>

class SWTypeExt;

// New SW Type framework. See SWTypes/*.h for examples of implemented ones. Don't touch yet, still WIP.
class NewSWType
{
	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType) {
		pType->TypeIndex = static_cast<int>(Array.size());
		Array.push_back(std::move(pType));
	}

	int TypeIndex;

public:
	NewSWType() : TypeIndex(-1) {
	}

	virtual ~NewSWType() {
	};

	virtual bool CanFireAt(SWTypeExt::ExtData *pSWType, const CellStruct &Coords) {
		return pSWType->CanFireAt(Coords);
	}

	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) {
		return false;
	}

	virtual bool Activate(SuperClass* pSW, const CellStruct &Coords, bool IsPlayer) = 0;

	virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW) {
	}

	virtual void LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI) {
	}

	virtual const char* GetTypeString() {
		return "";
	}

	int GetTypeIndex() {
		return TypeIndex;
	}

	virtual bool HandlesType(int type) {
		return false;
	}

	virtual SuperWeaponFlags::Value Flags() {
		return SuperWeaponFlags::None;
	}

	// static methods
	static void Init();

	static NewSWType* GetNthItem(int i) {
		return Array.at(i - FIRST_SW_TYPE).get();
	}

	static int FindIndex(const char* pType);

	static int FindHandler(int Type);
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine {
	static std::vector<std::unique_ptr<SWStateMachine>> Array;

public:
	SWStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: Type(pSWType), Super(pSuper), Coords(XY)
	{
		Clock.Start(Duration);
	}

	virtual ~SWStateMachine() {
	}

	virtual bool Finished() {
		return Clock.IsDone();
	}

	virtual void Update() {
	};

	virtual void PointerGotInvalid(void *ptr) {
	};

	int TimePassed() {
		return Unsorted::CurrentFrame - Clock.StartTime;
	}

	SWTypeExt::ExtData * FindExtData () {
		return SWTypeExt::ExtMap.Find(this->Super->Type);
	}

	// static methods
	static void Register(std::unique_ptr<SWStateMachine> Machine) {
		if(Machine) {
			Array.push_back(std::move(Machine));
		}
	}

	static void UpdateAll();

	static void InvalidatePointer(void *ptr);

	static void ClearAll();

protected:
	TimerStruct Clock;
	SuperClass* Super;
	NewSWType* Type;
	CellStruct Coords;
};

class UnitDeliveryStateMachine : public SWStateMachine {
public:
	UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{};

	virtual void Update();

	void PlaceUnits();
};

class ChronoWarpStateMachine : public SWStateMachine {
public:
	struct ChronoWarpContainer {
	public:
		BuildingClass* pBld;
		CellStruct target;
		CoordStruct origin;
		bool isVehicle;

		ChronoWarpContainer(BuildingClass* pBld, CellStruct target, CoordStruct origin, bool isVehicle) :
			pBld(pBld),
			target(target),
			origin(origin),
			isVehicle(isVehicle)
		{
		}

		ChronoWarpContainer() {
		}

		bool operator == (const ChronoWarpContainer &t) const {
			return (this->pBld == t.pBld);
		}
	};

	ChronoWarpStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType, DynamicVectorClass<ChronoWarpContainer> *Buildings)
		: SWStateMachine(Duration, XY, pSuper, pSWType)
	{
		for(int i=0; i<Buildings->Count; ++i) {
			this->Buildings.AddItem(Buildings->GetItem(i));
		}
		this->Duration = Duration;
	};

	virtual void Update();

	virtual void PointerGotInvalid(void *ptr);

protected:
	DynamicVectorClass<ChronoWarpContainer> Buildings;
	int Duration;
};

class PsychicDominatorStateMachine : public SWStateMachine {
public:
	PsychicDominatorStateMachine(CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
		: SWStateMachine(MAXINT32, XY, pSuper, pSWType)
	{
		PsyDom::Status = PsychicDominatorStatus::FirstAnim;

		// the initial deferment
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);
		this->Deferment = pData->SW_Deferment;

		// make the game happy
		PsyDom::Owner = pSuper->Owner;
		PsyDom::Coords = XY;
		PsyDom::Anim = nullptr;
	};

	virtual void Update();

protected:
	int Deferment;
};

#endif

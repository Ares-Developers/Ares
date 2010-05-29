#ifndef NEW_SW_TYPE_H
#define NEW_SW_TYPE_H

#include "../Ext/SWType/Body.h"

class SWTypeExt;

// New SW Type framework. See SWTypes/*.h for examples of implemented ones. Don't touch yet, still WIP.
class NewSWType
{
	protected:
		int TypeIndex;
		bool Registered;

		void Register()
			{ Array.AddItem(this); this->TypeIndex = Array.Count; }

	public:
		NewSWType()
			{ Registered = 0; Register(); };

		virtual ~NewSWType()
			{ };

		static void Init();

		virtual bool CanFireAt(CellStruct* pCoords)
			{ return 1; }
		virtual bool Launch(SuperClass* pSW, CellStruct* pCoords, byte IsPlayer) = 0;

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData,
			SuperWeaponTypeClass *pSW, CCINIClass *pINI) = 0;

		virtual const char * GetTypeString()
			{ return ""; }
		virtual const int GetTypeIndex()
			{ return TypeIndex; }

	static DynamicVectorClass<NewSWType *> Array;

	static NewSWType * GetNthItem(int i)
		{ return Array.GetItem(i - FIRST_SW_TYPE); }

	static int FindIndex(const char *Type) {
		for(int i = 0; i < Array.Count; ++i) {
			if(!strcmp(Array.GetItem(i)->GetTypeString(), Type)) {
				return FIRST_SW_TYPE + i;
			}
		}
		return -1;
	}
};

// state machines - create one to use delayed effects [create a child class per NewSWType, obviously]
// i.e. start anim/sound 1 frame after clicking, fire a damage wave 25 frames later, and play second sound 50 frames after that...
class SWStateMachine {
	public:
		static DynamicVectorClass<SWStateMachine *> Array;
	protected:
		TimerStruct  Clock;
		SuperClass * Super;
		NewSWType  * Type;
		CellStruct   Coords;
	public:
		bool Finished() { return Clock.IsDone(); }

		int TimePassed() { return Unsorted::CurrentFrame - Clock.StartTime; }

		SWStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
			: Type(pSWType), Super(pSuper), Coords(XY) {
			Clock.Start(Duration);
			Array.AddItem(this);
		}

		virtual ~SWStateMachine() {
			auto t = this;
			Array.RemoveItem(Array.FindItemIndex(&t));
		}

		virtual void Update() {};

		static void UpdateAll();

		SWTypeExt::ExtData * FindExtData () {
			return SWTypeExt::ExtMap.Find(this->Super->Type);
		}
};

class UnitDeliveryStateMachine : public SWStateMachine {
	public:
		UnitDeliveryStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType)
			: SWStateMachine(Duration, XY, pSuper, pSWType) {};
		virtual void Update();

		void PlaceUnits();
};

#endif

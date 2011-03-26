#ifndef NEW_SW_TYPE_H
#define NEW_SW_TYPE_H

#include "../Ext/SWType/Body.h"
#include "../Utilities/Enums.h"

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

		virtual bool CanFireAt(SWTypeExt::ExtData *pSWType, CellStruct* pCoords)
			{ return pSWType->CanFireAt(pCoords); }

		virtual bool AbortFire(SuperClass* pSW, bool IsPlayer)
			{ return false; }

		virtual bool Launch(SuperClass* pSW, CellStruct* pCoords, byte IsPlayer) = 0;

		virtual void Initialize(
			SWTypeExt::ExtData *pData,
			SuperWeaponTypeClass *pSW)
			{ }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData,
			SuperWeaponTypeClass *pSW, CCINIClass *pINI)
			{ }

		virtual const char * GetTypeString()
			{ return ""; }

		virtual const int GetTypeIndex()
			{ return TypeIndex; }

		virtual bool HandlesType(int type)
			{ return false; }

		virtual SuperWeaponFlags::Value Flags()
			{ return SuperWeaponFlags::None; }

	static DynamicVectorClass<NewSWType *> Array;

	static NewSWType * GetNthItem(int i)
		{ return Array.GetItem(i - FIRST_SW_TYPE); }

	static int FindIndex(const char *Type) {
		for(int i = 0; i < Array.Count; ++i) {
			if(Array.GetItem(i)->GetTypeString()) {
				if(!strcmp(Array.GetItem(i)->GetTypeString(), Type)) {
					return FIRST_SW_TYPE + i;
				}
			}
		}
		return -1;
	}

	static int FindHandler(int Type) {
		for(int i=0; i<Array.Count; ++i) {
			NewSWType *swt = Array.GetItem(i);
			if(swt->HandlesType(Type)) {
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

		virtual void PointerGotInvalid(void *ptr) {};

		static void UpdateAll();

		static void InvalidatePointer(void *ptr);

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

			ChronoWarpContainer() {}

			bool operator == (ChronoWarpContainer &t)
				{ return (this->pBld == t.pBld); }
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
			: SWStateMachine(MAXINT32, XY, pSuper, pSWType) {
				PsyDom::Status(PsychicDominatorStatus::FirstAnim);

				// the initial deferment
				SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);
				this->Deferment = pData->SW_Deferment.Get();

				// make the game happy
				PsyDom::Owner(pSuper->Owner);
				PsyDom::Coords(XY);
				PsyDom::Anim(NULL);
		};

		virtual void Update();

	protected:
		int Deferment;
};

#endif

#ifndef SUPERTYPE_EXT_CHRONOWARP_H
#define SUPERTYPE_EXT_CHRONOWARP_H

#include "../SWTypes.h"

class SW_ChronoWarp : public NewSWType
{
	public:
		SW_ChronoWarp() : NewSWType()
			{ };

		virtual ~SW_ChronoWarp()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		typedef ChronoWarpStateMachine TStateMachine;

		void newStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType,
			DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> *Buildings) {
				SWStateMachine::Register(std::make_unique<ChronoWarpStateMachine>(Duration, XY, pSuper, this, Buildings));
		}
};
#endif

#ifndef SUPERTYPE_EXT_CHRONOWARP_H
#define SUPERTYPE_EXT_CHRONOWARP_H

#include <xcompile.h>

#include "../SWTypes.h"

class SW_ChronoWarp : NewSWType
{
	public:
		SW_ChronoWarp() : NewSWType()
			{ };

		virtual ~SW_ChronoWarp()
			{ };

		virtual const char * GetTypeString()
			{ return NULL; }

		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		typedef ChronoWarpStateMachine TStateMachine;

		void newStateMachine(int Duration, CellStruct XY, SuperClass *pSuper, NewSWType * pSWType,
			DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> *Buildings) {
				new TStateMachine(Duration, XY, pSuper, this, Buildings);
		}
};
#endif

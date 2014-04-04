#ifndef SUPERTYPE_EXT_DOMINATOR_H
#define SUPERTYPE_EXT_DOMINATOR_H

#include "../SWTypes.h"

class SW_PsychicDominator : public NewSWType
{
	public:
		SW_PsychicDominator() : NewSWType()
			{ };

		virtual ~SW_PsychicDominator()
			{ };

		virtual const char * GetTypeString()
			{ return nullptr; }

		virtual void LoadFromINI(
			SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
		virtual void Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW);
		virtual bool AbortFire(SuperClass* pSW, bool IsPlayer);
		virtual bool Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer);
		virtual bool HandlesType(int type);
		virtual SuperWeaponFlags::Value Flags();

		virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const override;
		virtual int GetDamage(const SWTypeExt::ExtData* pData) const override;
		virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const override;

		static SuperClass* CurrentPsyDom;

		typedef PsychicDominatorStateMachine TStateMachine;

		void newStateMachine(CellStruct XY, SuperClass *pSuper) {
			SWStateMachine::Register(std::make_unique<PsychicDominatorStateMachine>(XY, pSuper, this));
		}
};
#endif

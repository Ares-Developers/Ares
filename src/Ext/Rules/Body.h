#ifndef RULES_EXT_H
#define RULES_EXT_H

#include <CCINIClass.h>
#include <WeaponTypeClass.h>
#include <RulesClass.h>
#include <AnimTypeClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

//ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
//endif

class RulesExt
{
	public:
	typedef RulesClass TT;

	class ExtData : public Extension<TT>
	{
		public:
		Valueable<AnimTypeClass* >ElectricDeath;
		double EngineerDamage;
		bool EngineerAlwaysCaptureTech;
		Valueable<MouseCursor> EngineerDamageCursor;
		bool MultiEngineer[2];

		Valueable<AnimTypeClass *> FirestormActiveAnim, FirestormIdleAnim, FirestormGroundAnim, FirestormAirAnim;

		Customizable<WarheadTypeClass *> FirestormWH;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			ElectricDeath(NULL),
			EngineerDamage (0.0F),
			EngineerAlwaysCaptureTech (true),
			EngineerDamageCursor (MouseCursor::First[51]),
			FirestormActiveAnim(NULL),
			FirestormIdleAnim(NULL),
			FirestormGroundAnim(NULL),
			FirestormAirAnim(NULL),
			FirestormWH(&RulesClass::Instance->C4Warhead)
			{
				MultiEngineer[0] = false; // Skirmish
				MultiEngineer[1] = false; // LAN
				MultiEngineer[2] = false; // WOnline
			};

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadBeforeTypeData(TT *pThis, CCINIClass *pINI);
		virtual void LoadAfterTypeData(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);

		virtual void InvalidatePointer(void *ptr) {
		}
};

private:
	static ExtData *Data;

public:
	static void Allocate(RulesClass *pThis);
	static void Remove(RulesClass *pThis);

	static void LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI);
	static void LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI);
	static void LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI);

	static ExtData* Global()
	{
		return Data;
	};

};

#endif

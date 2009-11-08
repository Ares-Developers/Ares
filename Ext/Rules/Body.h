#ifndef RULES_EXT_H
#define RULES_EXT_H

#include <CCINIClass.h>
#include <WeaponTypeClass.h>
#include <RulesClass.h>
#include <AnimTypeClass.h>

#include "..\_Container.hpp"
#include "..\..\Helpers\Template.h"

//ifdef DEBUGBUILD
#include "..\..\Misc\Debug.h"
//endif

class RulesExt
{
	public:
	typedef RulesClass TT;

	class ExtData : public Extension<TT> 
	{
		public:
		Valueable<AnimTypeClass* >ElectricDeath;

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
			ElectricDeath(NULL)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadBeforeTypeData(TT *pThis, CCINIClass *pINI);
		virtual void LoadAfterTypeData(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);
		virtual void Initialize(TT *pThis);
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

#ifndef ARMORS_H
#define ARMORS_H

#include <ArrayClasses.h>
#include <CCINIClass.h>
#include <WarheadTypeClass.h>

#include <Conversions.h>

#include "../Ext/WarheadType/Body.h"
#include "../Ares.h"
#include "../Ares.CRT.h"

#include "_Enumerator.hpp"

class ArmorType;

class ArmorType : public Enumerable<ArmorType>
{
public:
	ArmorType(const char *Title) : Enumerable<ArmorType>(Title), DefaultIndex(-1) { }

	virtual ~ArmorType() override = default;

	virtual void LoadFromINI(CCINIClass *pINI) override;

	virtual void LoadFromStream(AresStreamReader &Stm) override;

	virtual void SaveToStream(AresStreamWriter &Stm) override;

	static void LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH);
	static void AddDefaults();

	int DefaultIndex;
	WarheadTypeExt::VersesData DefaultVerses;
};

#endif

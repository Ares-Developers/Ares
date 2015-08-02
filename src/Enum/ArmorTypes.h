#pragma once

#include "_Enumerator.hpp"
#include "../Ext/WarheadType/Body.h"

class CCINIClass;
class WarheadTypeClass;

class ArmorType final : public Enumerable<ArmorType>
{
public:
	ArmorType(const char* pTitle);

	virtual ~ArmorType() override;

	virtual void LoadFromINI(CCINIClass *pINI) override;

	virtual void LoadFromStream(AresStreamReader &Stm) override;

	virtual void SaveToStream(AresStreamWriter &Stm) override;

	static void LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH);
	static void AddDefaults();

	int DefaultIndex;
	WarheadTypeExt::VersesData DefaultVerses;
};

#pragma once

#include "_Enumerator.hpp"

#include "../Utilities/Iterator.h"

class BuildingTypeClass;
class CCINIClass;
class HouseClass;
class TechnoTypeClass;

class GenericPrerequisite final : public Enumerable<GenericPrerequisite>
{
public:
	GenericPrerequisite(const char* pTitle);

	virtual ~GenericPrerequisite() override;

	virtual void LoadFromINI(CCINIClass *pINI) override;

	virtual void LoadFromStream(AresStreamReader &Stm) override;

	virtual void SaveToStream(AresStreamWriter &Stm) override;

	static void AddDefaults();

	DynamicVectorClass<int> Prereqs;
	DynamicVectorClass<TechnoTypeClass*> Alternates;
};

class Prereqs
{
public:
	using BTypeIter = Iterator<BuildingTypeClass*>;

	static void Parse(CCINIClass *pINI, const char* section, const char *key, DynamicVectorClass<int> &Vec);
	static void ParseAlternate(CCINIClass *pINI, const char* section, const char *key, DynamicVectorClass<TechnoTypeClass*> &Vec);

	static bool HouseOwnsGeneric(HouseClass const* pHouse, int Index);
	static bool HouseOwnsSpecific(HouseClass const* pHouse, int Index);
	static bool HouseOwnsPrereq(HouseClass const* pHouse, int Index);

	static bool HouseOwnsAll(HouseClass const* pHouse, const DynamicVectorClass<int> &list);
	static bool HouseOwnsAny(HouseClass const* pHouse, const DynamicVectorClass<int> &list);

	static bool ListContainsGeneric(const BTypeIter &List, int Index);
	static bool ListContainsSpecific(const BTypeIter &List, int Index);
	static bool ListContainsPrereq(const BTypeIter &List, int Index);

	static bool ListContainsAll(const BTypeIter &List, const DynamicVectorClass<int> &Requirements);
	static bool ListContainsAny(const BTypeIter &List, const DynamicVectorClass<int> &Requirements);
};

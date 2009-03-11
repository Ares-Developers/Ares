#ifndef NEW_SW_TYPE_H
#define NEW_SW_TYPE_H

#include "..\Ext\SWType\Body.h"

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
		virtual bool Launch(SuperClass* pSW, CellStruct* pCoords) = 0;

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

#endif

#ifndef BulletType_EXT_H
#define BulletType_EXT_H

#include <CCINIClass.h>
#include <BulletTypeClass.h>

#include <Helpers\Macro.h>

#include "..\_Container.hpp"
#include "..\..\Ares.h"

#include "..\..\Misc\Debug.h"

#define FOUNDATION_CUSTOM	0x7F

class BulletTypeExt
{
public:
	typedef BulletTypeClass TT;

	class ExtData : public Extension<TT> 
	{
	public:
		// solid
		bool SubjectToSolid;
		
		// firewall
		bool SubjectToFirewall;

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) :
			SubjectToSolid (false),
			SubjectToFirewall (true)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINI(TT *pThis, CCINIClass *pINI);
	};

	static Container<BulletTypeExt> ExtMap;
//	static ExtData ExtMap;
};

#endif

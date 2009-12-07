#ifndef BULLETTYPE_EXT_H
#define BULLETTYPE_EXT_H

#include <CCINIClass.h>
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

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

		bool Parachuted;

		// added on 11.11.09 for #667 (part of Trenches)
		bool SubjectToTrenches; //! if false, this projectile/weapon *always* passes through to the occupants, regardless of UC.PassThrough

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
			SubjectToSolid (false),
			SubjectToFirewall (true),
			Parachuted (false),
			SubjectToTrenches (true)
			{ };

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
	};

	static Container<BulletTypeExt> ExtMap;
//	static ExtData ExtMap;
};

#endif

#ifndef TIBERIUM_EXT_H
#define TIBERIUM_EXT_H

#include <xcompile.h>
#include <TiberiumClass.h>

#include "../../Utilities/Template.h"

#include "../_Container.hpp"

class TiberiumExt
{
	public:
	typedef TiberiumClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject)
		{
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}
	};

	static Container<TiberiumExt> ExtMap;
};

#endif

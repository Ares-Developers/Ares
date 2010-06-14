#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "../../Ares.h"
#include "../../Utilities/Template.h"
#include "../_Container.hpp"

#include <xcompile.h>
#include <CCINIClass.h>
#include <ColorScheme.h>
#include <VocClass.h>
#include <CampaignClass.h>

class CampaignExt
{
	public:
	typedef CampaignClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		bool DebugOnly;
		char HoverSound[0x1F];
		char Summary[0x20];

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			DebugOnly (false)
		{
			*HoverSound = 0;
			*Summary = 0;
		};

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr) {
		}

		bool isVisible() {
			return !this->DebugOnly || Ares::UISettings::ShowDebugCampaigns;
		}
	};

	static Container<CampaignExt> ExtMap;
	static DynamicVectorClass<CampaignExt::ExtData*> Array;

	static int lastSelectedCampaign;

	static int countVisible() {
		int ret = 0;
		for(int i=0; i<Array.Count; ++i) {
			if(Array.GetItem(i)->isVisible()) {
				++ret;
			}
		}
		return ret;
	}
};

#endif

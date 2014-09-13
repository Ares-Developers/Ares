#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "../../Ares.h"
#include "../../Utilities/Constructs.h"
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
		AresFixedString<0x20> HoverSound;
		Valueable<CSFText> Summary;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			DebugOnly(false)
		{ };

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr, bool bRemoved) { }

		bool IsVisible() const {
			return !this->DebugOnly || Ares::UISettings::ShowDebugCampaigns;
		}
	};

	static Container<CampaignExt> ExtMap;
	static DynamicVectorClass<CampaignExt::ExtData*> Array;

	static int lastSelectedCampaign;

	static int CountVisible();
};

#endif

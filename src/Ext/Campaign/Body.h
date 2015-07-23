#pragma once

#include "../../Ares.h"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"
#include "../_Container.hpp"

#include <CCINIClass.h>
#include <ColorScheme.h>
#include <VocClass.h>
#include <CampaignClass.h>

class CampaignExt
{
public:
	using base_type = CampaignClass;

	class ExtData final : public Extension<CampaignClass>
	{
	public:
		Valueable<bool> DebugOnly;
		AresFixedString<0x20> HoverSound;
		Valueable<CSFText> Summary;

		ExtData(CampaignClass* OwnerObject) : Extension<CampaignClass>(OwnerObject),
			DebugOnly(false)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override { }

		bool IsVisible() const {
			return !this->DebugOnly || Ares::UISettings::ShowDebugCampaigns;
		}
	};

	class ExtContainer final : public Container<CampaignExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static DynamicVectorClass<CampaignExt::ExtData*> Array;

	static int lastSelectedCampaign;

	static int CountVisible();
};

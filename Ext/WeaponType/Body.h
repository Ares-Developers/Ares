#ifndef WEAPONTYPE_EXT_H
#define WEAPONTYPE_EXT_H

#include <Helpers\Macro.h>
#include <Helpers\Template.h>
#include <BombClass.h>
#include <BombListClass.h>
#include <BulletClass.h>
#include <CCINIClass.h>
#include <Drawing.h>
#include <EBolt.h>
#include <RadBeam.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <WarheadTypeClass.h>
#include <WaveClass.h>
#include <WeaponTypeClass.h>
#include <ScenarioClass.h>
#include <MouseClass.h>

#include "..\..\Enum\RadTypes.h"

#include "..\..\Misc\Debug.h"

#include "..\_Container.hpp"

#include "..\..\Helpers\Template.h"

struct SHPStruct;

class WeaponTypeExt
{
public:
	typedef WeaponTypeClass TT;
	class ExtData : public Extension<TT> 
	{
	public:
		// Generic
		bool Weapon_Loaded;
		WeaponTypeClass *Weapon_Source;

		// Coloured Rad Beams
		Customizable<ColorStruct> Beam_Color;
		int    Beam_Duration;
		double Beam_Amplitude;
		bool   Beam_IsHouseColor;

		// Coloured EBolts
		Customizable<ColorStruct> Bolt_Color1;
		Customizable<ColorStruct> Bolt_Color2;
		Customizable<ColorStruct> Bolt_Color3;

		// TS Lasers
		bool   Wave_IsHouseColor;
		bool   Wave_IsLaser;
		bool   Wave_IsBigLaser;
		Customizable<ColorStruct> Wave_Color;
		bool   Wave_Reverse[5];
/*
		int    Wave_InitialIntensity;
		int    Wave_IntensityStep;
		int    Wave_FinalIntensity;
*/
		// custom Ivan Bombs
		bool Ivan_KillsBridges;
		bool Ivan_Detachable;
		Customizable<int> Ivan_Damage;
		Customizable<int> Ivan_Delay;
		Customizable<int> Ivan_TickingSound;
		Customizable<int> Ivan_AttachSound;
		Customizable<WarheadTypeClass *> Ivan_WH;
		Customizable<SHPStruct *> Ivan_Image;
		Customizable<int> Ivan_FlickerRate;

		RadType * Rad_Type;

//		MouseCursor Cursor_Attack;
//		bool Cursor_Custom;

		ExtData(const DWORD Canary = 0) :
			Weapon_Loaded (false),
			Weapon_Source (NULL),
			Beam_IsHouseColor (false),
			Beam_Duration (15),
			Beam_Amplitude (40.0),
			Wave_IsHouseColor (false),
			Wave_IsLaser (false),
			Wave_IsBigLaser (false),
			Ivan_KillsBridges (true),
			Ivan_Detachable (true),
			Ivan_Damage (&RulesClass::Global()->IvanDamage),
			Ivan_Delay (&RulesClass::Global()->IvanTimedDelay),
			Ivan_TickingSound (&RulesClass::Global()->BombTickingSound),
			Ivan_AttachSound (&RulesClass::Global()->BombAttachSound),
			Ivan_WH (&RulesClass::Global()->IvanWarhead),
			Beam_Color (RulesClass::Global()->get_RadColor()),
			Wave_Color (NULL),
			Bolt_Color1 (NULL),
			Bolt_Color2 (NULL),
			Bolt_Color3 (NULL),
			Ivan_Image (&RulesClass::Global()->BOMBCURS_SHP),
			Ivan_FlickerRate (&RulesClass::Global()->IvanIconFlickerRate),
			Rad_Type (NULL)
			{
//				this->Beam_Color = ColorStruct(255, 255, 255);
//				this->Wave_Color = ColorStruct(255, 255, 255);
				for(int i = 0; i < 5; ++i) {
					this->Wave_Reverse[i] = false;
				}
			};

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINI(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT* pThis);
	};

/*
	struct WeaponWeight
	{
		short index;
		bool InRange;
		float DPF;
		bool operator < (const WeaponWeight &RHS) const {
			return (this->InRange < RHS.InRange && this->DPF < RHS.DPF);
		}
	};

	EXT_P_DECLARE(WeaponTypeClass);
	EXT_FUNCS(WeaponTypeClass);
	EXT_INI_FUNCS(WeaponTypeClass);

*/

	static Container<WeaponTypeExt> ExtMap;

	static stdext::hash_map<BombClass *, ExtData *> BombExt;
	static stdext::hash_map<WaveClass *, ExtData *> WaveExt;
	static stdext::hash_map<EBolt *, ExtData *> BoltExt;
	static stdext::hash_map<RadSiteClass *, ExtData *> RadSiteExt;

#define idxVehicle 0
#define idxAircraft 1
#define idxBuilding 2
#define idxInfantry 3
#define idxOther 4

	static char AbsIDtoIdx(eAbstractType absId)
		{
			switch(absId)
			{
				case abs_Unit:
					return idxVehicle;
				case abs_Aircraft:
					return idxAircraft;
				case abs_Building:
					return idxBuilding;
				case abs_Infantry:
					return idxInfantry;
				default:
					return idxOther;
			}
		};

	static void ModifyWaveColor(WORD *src, WORD *dst, int Intensity, WaveClass *Wave);

	static void PointerGotInvalid(void *ptr);
};

typedef stdext::hash_map<BombClass *, WeaponTypeExt::ExtData *> hash_bombExt;
typedef stdext::hash_map<WaveClass *, WeaponTypeExt::ExtData *> hash_waveExt;
typedef stdext::hash_map<EBolt *, WeaponTypeExt::ExtData *> hash_boltExt;
typedef stdext::hash_map<RadSiteClass *, WeaponTypeExt::ExtData *> hash_radsiteExt;

#endif

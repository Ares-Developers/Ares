#pragma once

#include <MouseClass.h>

class Actions
{
	public:
		// actions for custom sw
		static const Action SuperWeaponAllowed = static_cast<Action>(0x7F);
		static const Action SuperWeaponDisallowed = static_cast<Action>(0x7E);

		static void Set(MouseCursor *pCursor, bool bAllowShroud = false);

		static MouseCursor MP;
		static MouseCursor* MPCurrent;
		static MouseCursor* MPCustom;

		static MouseCursor* TempCursor;

		static bool MPCustomAllowShroud;
};

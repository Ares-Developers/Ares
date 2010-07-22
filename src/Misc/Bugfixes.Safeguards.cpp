#include <IsometricTileTypeClass.h>

#include "Debug.h"
#include "../Ares.h"

DEFINE_HOOK(547043, IsometricTileTypeClass_ReadFromFile, 6)
{
	GET(int, FileSize, EBX);
	GET(IsometricTileTypeClass *, pTileType, ESI);

	if(FileSize == 0) {
		const char * filename = pTileType->FileName;
		if(strlen(filename) >= 14) {
			Debug::FatalError("Maximum allowed length for tile names, excluding the extension, is 9 characters."
					"The tile called '%s' exceeds this limit - the game cannot proceed.", filename);
		} else {
			Debug::FatalError("The tile called '%s' could not be loaded for some reason - make sure the file exists.", filename);
		}
	}
	return 0;
}

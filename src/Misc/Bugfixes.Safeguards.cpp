#include <IsometricTileTypeClass.h>

#include "Debug.h"
#include "../Ares.h"

DEFINE_HOOK(547043, IsometricTileTypeClass_ReadFromFile, 6)
{
	GET(int, FileSize, EBX);
	GET(IsometricTileTypeClass *, pTileType, ESI);

	if(FileSize == 0) {
		const char * tile = pTileType->ID;
		if(strlen(tile) > 9) {
			Debug::FatalErrorAndExit("Maximum allowed length for tile names, excluding the extension, is 9 characters.\n"
					"The tileset using filename '%s' exceeds this limit - the game cannot proceed.", tile);
		} else {
			Debug::FatalErrorAndExit("The tileset '%s' contains a file that could not be loaded for some reason - make sure the file exists.", tile);
		}
	}
	return 0;
}

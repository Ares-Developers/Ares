#ifndef CONSTRUCTS_H_
#define CONSTRUCTS_H_

// custom paletted cameos
// TODO: add a static vector to buffer instances of the same Palette file?
#include <ConvertClass.h>
#include <CCINIClass.h>
#include <GeneralStructures.h>

#include "Ares.h"

class CustomPalette {
public:
	ConvertClass *Convert;
	BytePalette *Palette;

	CustomPalette() :
		Convert(NULL),
		Palette(NULL)
	{};

	~CustomPalette() {
		delete this->Convert;
		delete this->Palette;
	}

	bool LoadFromINI(CCINIClass *pINI, const char *pSection, const char *pKey) {
		if(pINI->ReadString(pSection, pKey, "", Ares::readBuffer, Ares::readLength)) {
			delete this->Palette;
			delete this->Convert;
			ConvertClass::CreateFromFile(Ares::readBuffer, &this->Palette, &this->Convert);
			return true;
		}
		return false;
	};
};

#endif

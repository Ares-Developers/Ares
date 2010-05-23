#ifndef INIITERATORCHAR_H
#define INIITERATORCHAR_H

#include <CCINIClass.h>
#include <CRT.h>
#include "Debug.h"

class IniIteratorChar
{
public:
	static const char iteratorChar[];
	static const char iteratorReplacementFormat[];

	static int iteratorValue;

	static char buffer[];
};

#endif

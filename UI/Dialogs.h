#ifndef ARES_DIALOGS_H
#define ARES_DIALOGS_H
#include "registered.h"

class Dialogs {
public:
	static const char *StatusString;
	static wchar_t ExceptDetailedMessage[0x400];
	static const int ExceptControlID;
};

#endif

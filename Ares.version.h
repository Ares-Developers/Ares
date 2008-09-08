#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 58
#define VERSION_BUILD 1

// ffs macro hell
#define str(x) str_(x)
#define str_(x) #x

#define VERSION_PREFIX "Yuri's Revenge 1.001 + Ares version "
#define VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION) "." str(VERSION_BUILD)
#define VERSION_STRING VERSION_PREFIX VERSION_STR
#define VERSION_STRVER "Ares version: " VERSION_STR
#endif

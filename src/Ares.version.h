#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 995

#define SAVEGAME_MAGIC ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION))

#define str(x) str_(x)
#define str_(x) #x

#define VERSION_PREFIX "Yuri's Revenge 1.001 + Ares version "
#define VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION)

// "Yuri's Revenge 1.001 + Ares version: $ver"
#define VERSION_STRING VERSION_PREFIX VERSION_STR

// "Ares version: $ver"
#define VERSION_STRVER "Ares version: " VERSION_STR

// "Ares/$ver"
#define VERSION_STREX "Ares/" VERSION_STR

// "1.001/Ares $ver"
#define VERSION_STRMINI "1.001/Ares " VERSION_STR

#define VERSION_INTERNAL "Ares r" str(VERSION_REVISION)

#endif

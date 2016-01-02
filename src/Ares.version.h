#pragma once

// define this to switch to release version
//#define IS_RELEASE_VER

#define VERSION_MAJOR 16
#define VERSION_MINOR 2
#define VERSION_REVISION 813

#define SAVEGAME_MAGIC ((VERSION_MAJOR << 24) | (VERSION_MINOR << 12) | (VERSION_REVISION))

#define wstr(x) wstr_(x)
#define wstr_(x) L ## #x
#define str(x) str_(x)
#define str_(x) #x

#define VERSION_PREFIX "Yuri's Revenge 1.001 + Ares version "
#define VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION)
#define VERSION_WSTR wstr(VERSION_MAJOR) L"." wstr(VERSION_MINOR) L"." wstr(VERSION_REVISION)

// Alternative version display name for release versions
#ifdef IS_RELEASE_VER

#define PRODUCT_MAJOR 0
#define PRODUCT_MINOR 10
#define PRODUCT_REVISION 0
#define PRODUCT_STR "0.A"
#define DISPLAY_STR PRODUCT_STR

#else

#define PRODUCT_MAJOR VERSION_MAJOR
#define PRODUCT_MINOR VERSION_MINOR
#define PRODUCT_REVISION VERSION_REVISION
#define PRODUCT_STR VERSION_STR
#define DISPLAY_STR VERSION_STR

#endif // IS_RELEASE_VER

// "Yuri's Revenge 1.001 + Ares version: $ver"
#define VERSION_STRING VERSION_PREFIX VERSION_STR
#define DISPLAY_STRING VERSION_PREFIX DISPLAY_STR

// "Ares version: $ver"
#define VERSION_STRVER "Ares version: " VERSION_STR
#define DISPLAY_STRVER "Ares version: " DISPLAY_STR

// "Ares/$ver"
#define VERSION_STREX "Ares/" VERSION_STR
#define DISPLAY_STREX "Ares/" DISPLAY_STR

// "1.001/Ares $ver"
#define VERSION_STRMINI "1.001/Ares " VERSION_STR
#define DISPLAY_STRMINI "1.001/Ares " DISPLAY_STR

#define VERSION_INTERNAL "Ares r" VERSION_STR

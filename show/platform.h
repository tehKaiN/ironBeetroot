#ifndef GUARD_SHOW_PLATFORM_H
#define GUARD_SHOW_PLATFORM_H

typedef struct _tShowPlatform{
	UBYTE ubId;
  UBYTE ubX;              /// X coord
  UBYTE ubY;              /// Y coord
  UBYTE ubType;           /// See PLATFORM_* macros
  tShowPackage *pPackage; /// Currently held package
} tShowPlatform;

#endif // GUARD_SHOW_PLATFORM_H


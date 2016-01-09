#ifndef GUARD_SYMULACJA_PLATFORM_H
#define GUARD_SYMULACJA_PLATFORM_H

#include "../common/types.h"
#include "package.h"

#define PLATFORM_UNK 0
#define PLATFORM_IN 1
#define PLATFORM_OUT 2
#define PLATFORM_HELPER 3

typedef struct _tPlatform{
	UBYTE ubId;
  UBYTE ubFieldX;
  UBYTE ubFieldY;
  UBYTE ubType;       /// See PLATFORM_* macros
  tPackage *pPackage; /// Currently laying package, 0 if empty
} tPlatform;

void platformInit(
	IN UBYTE ubIdx,
	IN UBYTE ubFieldX,
	IN UBYTE ubFieldY,
	IN UBYTE ubType
);

#endif // GUARD_SYMULACJA_PLATFORM_H

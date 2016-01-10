#ifndef GUARD_HALL_PACKAGE_H
#define GUARD_HALL_PACKAGE_H

#include "../common/types.h"
#include "platform.h"

typedef struct _tPackage{
	ULONG ulIdx;              /// Current package number
	UBYTE ubPriority;         /// Higher value == higher priority
	struct _tPlatform *pDest; /// Destination platform
} tPackage;

tPackage *packageCreate(
	IN struct _tPlatform *pPlatform,
	IN UBYTE ubPriority,
	IN UBYTE ubDestId
);

void packageDestroy(
	IN tPackage *pPackage
);

#endif // GUARD_HALL_PACKAGE_H


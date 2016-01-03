#ifndef GUARD_SYMULACJA_PACKAGE_H
#define GUARD_SYMULACJA_PACKAGE_H

#include "../common/types.h"

typedef struct _tPackage{
	ULONG ulIdx;              /// Current package number
	UBYTE ubPriority;         /// Higher value == higher priority
	struct _tPlatform *pDest; /// Destination platform
	char *szName;             /// Displayable name
} tPackage;

#endif // GUARD_SYMULACJA_PACKAGE_H


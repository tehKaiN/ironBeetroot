#ifndef GUARD_HALL_PLATFORM_H
#define GUARD_HALL_PLATFORM_H

#include "../common/types.h"
#include "../common/net/net.h"
#include "package.h"

#define PLATFORM_UNK 0
#define PLATFORM_IN 1
#define PLATFORM_OUT 2
#define PLATFORM_HELPER 3

typedef struct _tPlatform{
	UBYTE ubId;
  UBYTE ubFieldX;             /// Platform coords
  UBYTE ubFieldY;             /// ------||-------
  UBYTE ubType;               /// See PLATFORM_* macros
  struct _tPackage *pPackage; /// Currently laying package, 0 if empty
  tNetConn *pOwner;           /// Associated giveTake client
} tPlatform;

void platformInit(
	IN UBYTE ubIdx,
	IN UBYTE ubFieldX,
	IN UBYTE ubFieldY,
	IN UBYTE ubType
);

UBYTE platformReserveForClient(
	IN tNetConn *pClientConn,
	IN UBYTE ubType,
	OUT UBYTE *pId
);

tPlatform *platformGetByClient(
	IN tNetConn *pClientConn,
	IN UBYTE ubType
);

tPlatform *platformGetById(
	IN UBYTE ubId
);

#endif // GUARD_HALL_PLATFORM_H

#ifndef GUARD_LEADER_PLATFORM_H
#define GUARD_LEADER_PLATFORM_H

#define PLATFORM_UNK 0
#define PLATFORM_IN 1
#define PLATFORM_OUT 2
#define PLATFORM_HELPER 3

#include "package.h"
#include "arm.h"

typedef struct _tLeaderPlatform{
  UBYTE ubId;                       ///
  UBYTE ubX;                        ///
  UBYTE ubY;                        ///
  UBYTE ubType;                     ///
  struct _tLeaderPackage *pPackage; /// Currently laying package
	                                   // For helper only
  struct _tLeaderArm *pArmIn;       /// Arm, which can pick up package
  struct _tLeaderArm *pArmOut;      /// Arm, which can put down package
} tLeaderPlatform;

void platformAlloc(
	IN UBYTE ubPlatformCount
);

void platformFree(void);

tLeaderPlatform *platformGetById(
	IN UBYTE ubId
);

#endif // GUARD_LEADER_PLATFORM_H

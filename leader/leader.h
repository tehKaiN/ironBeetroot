#ifndef GUARD_LEADER_LEADER_H
#define GUARD_LEADER_LEADER_H

#include "../common/net/net.h"
#include "platform.h"
#include "arm.h"
#include "package.h"

#define READY_ID     1
#define READY_HALL   2

#define FIELD_PLATFORM   1
#define FIELD_ARMA       2
#define FIELD_ARMB       4
#define FIELD_RESERVEDA  8
#define FIELD_RESERVEDB 16

typedef struct _tLeader{
	                              // Network stuff
	tNetClient *pClient;         /// Hall client handle
	tNetServer *pServer;         /// Server for arms
	                              // Timers
	uv_timer_t sRouteTimer;      /// Routing timer
	uv_timer_t sHallTimer;       /// Hall status poll timer
	UBYTE ubReady;               /// See READY_* flags
	                              // Arm fields
	tLeaderArm *pArmA;           /// HE HE HE...
	tLeaderArm *pArmB;           /// Secondary (lower) arm
	                              // Platform fields
	tLeaderPlatform *pPlatforms; /// Platform array
	UBYTE ubPlatformCount;       /// Platform count
	                              // Field fields
	UBYTE *pFields;              /// 2D Field array
	UBYTE ubWidth;               /// Hall width
	UBYTE ubHeight;              /// Hall height
	                              // Package fields
	tLeaderPackage *pPackages;  /// Package array
	UBYTE ubPackageCount;       /// Package count
	uv_mutex_t sPackageMutex;   /// Mutex for pPackages & ubPackageCount
} tLeader;

void leaderCreate(void);

void leaderDestroy(void);

extern tLeader g_sLeader;

#endif // GUARD_LEADER_LEADER_H


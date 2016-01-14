#ifndef GUARD_ARM_ARM_H
#define GUARD_ARM_ARM_H

#include "../common/arm.h"
#include "../common/net/net.h"

#define READY_ID_HALL 1
#define READY_ID_LEADER 2

typedef struct _tArm{
	                            // Network stuff
	tNetClient *pClientHall;   /// Connection to Hall
	tNetClient *pClientLeader; /// Connection to Leader
	UBYTE ubReady;             /// See READY_* flags
	uv_timer_t sSensorTimer;   /// Sensor update timer
	UBYTE ubId;                /// See ARM_ID_* flags
														  // Sensor info
	UWORD uwCurrX;             /// Current coords
	UWORD uwCurrY;             /// ------||------
	UBYTE ubState;             /// See ARM_STATE_* flags
	uv_mutex_t sSensorMutex;   /// Sensor info mutex
	                            // Command fields
	UWORD uwDestX;             /// Destination coords
	UWORD uwDestY;             /// ------||----------
	UBYTE pCmds[MAX_COMMANDS]; /// Command array
	UBYTE ubCmdCurr;           /// Current cmd index
	UBYTE ubCmdCount;          /// Current command count
	UBYTE ubCmdState;          /// See ARM_CMDSTATE_* flags
	uv_mutex_t sCmdMutex;      /// Command mutex
} tArm;

void armCreate(void);

void armDestroy(void);

void armOnConnect(
	IN tNetClient *pClient
);

void armSensorUpdate(
	IN uv_timer_t *pTimer
);

extern tArm g_sArm;

#endif // GUARD_ARM_ARM_H


#ifndef GUARD_ARM_ARM_H
#define GUARD_ARM_ARM_H

#include "../common/arm.h"

#define READY_ID 1

typedef struct _tArm{
	                            // Network stuff
	tNetClient *pClient;       /// Connection to Hall
	UBYTE ubReady;             /// See READY_* flags
	uv_timer_t sSensorTimer;   /// Sensor update timer
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

void ramieProcessPacket(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void armOnConnect(
	IN tNetClient *pClient
);

void armCreate(void);

void armDestroy(void);

tArm g_sArm;

#endif // GUARD_ARM_ARM_H


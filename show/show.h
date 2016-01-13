#ifndef GUARD_SHOW_SHOW_H
#define GUARD_SHOW_SHOW_H

#include <uv.h>
#include "../common/types.h"
#include "../common/net/net.h"

#define READY_ID_HALL 1
#define READY_ID_LEADER 2

typedef struct _tShow{
	                           // Network stuff
	tNetClient *pClientHall;   /// Connection to Hall
	tNetClient *pClientLeader; /// Connection to Leader
  UBYTE ubReady;             /// Network ready state
                             // SDL
	uv_idle_t sSDLIdle;        /// Idle timer for SDL
} tShow;

void showCreate(void);

void showDestroy(void);

void showOnConnect(
	IN tNetClient *pClient
);

void showSDLUpdate(
	IN uv_idle_t *pIdle
);

extern tShow g_sShow;

#endif // GUARD_SHOW_SHOW_H

#ifndef GUARD_SHOW_SHOW_H
#define GUARD_SHOW_SHOW_H

#include <uv.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "../common/types.h"
#include "../common/net/net.h"
#include "package.h"
#include "customer.h"
#include "platform.h"
#include "arm.h"

#define READY_HALL_ID       1
#define READY_HALL_FIELDS   2
#define READY_HALL_PACKAGES 4
#define READY_HALL_ARMS     8

#define READY_LEADER_ID     1
#define READY_LEADER_ROUTEA 2
#define READY_LEADER_ROUTEB 4

typedef struct _tShow{
	                            // Network stuff
	tNetClient *pClientHall;   /// Connection to Hall
	tNetClient *pClientLeader; /// Connection to Leader
  UBYTE ubReadyHall;         /// Hall ready state
  UBYTE ubReadyLeader;       /// Leader ready state
                              // SDL
	uv_idle_t sSDLIdle;        /// Idle timer for SDL
	SDL_Window *pWindow;       /// Window handle
	SDL_Surface *pSurfWnd;     /// Window surface
	TTF_Font *pFont;
	SDL_Event sEvt;
	                            // Hall info
	UBYTE ubHallWidth;         /// Hall width
	UBYTE ubHallHeight;        /// Hall height
	                            // Packages
	tShowPackage *pPackages;   /// Package array
	UBYTE ubPackageCount;      /// Length of pPackages
	                            // Customers
	tShowCustomer *pCustomers; /// Customer array
	UBYTE ubCustomerCount;     /// Length of pCustomers
	                            // Platforms
	tShowPlatform *pPlatforms; /// Platform array
	UBYTE ubPlatformCount;     /// Length of pPlatforms
	                            // Arms
	tShowArm sArmA;
	tShowArm sArmB;
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

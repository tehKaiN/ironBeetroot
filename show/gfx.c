#include "gfx.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "show.h"
#include "../common/log.h"

void gfxCreate(void) {
	// Init
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	// SDL main
	g_sShow.pWindow = SDL_CreateWindow(
		"Show", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_SHOWN
	);
	g_sShow.pSurfWnd = SDL_GetWindowSurface(g_sShow.pWindow);
	if(!g_sShow.pWindow)
		logError("Can't create SDL window");

	// Font
	g_sShow.pFont = TTF_OpenFont("arial.ttf", 12);
	if(!g_sShow.pFont)
		logError("Can't load font: %s", TTF_GetError());

	uv_idle_init(g_sNetManager.pLoop, &g_sShow.sSDLIdle);
	uv_idle_start(&g_sShow.sSDLIdle, gfxIdle);
}

void gfxDestroy(void) {
	// Stop event loop
	uv_idle_stop(&g_sShow.sSDLIdle);

	// Close stuff
	TTF_CloseFont(g_sShow.pFont);
	SDL_DestroyWindow(g_sShow.pWindow);

	// Quit
	TTF_Quit();
	SDL_Quit();
}

void gfxUpdate(void) {
	static double t = 0;

	g_sShow.pSurfWnd = SDL_GetWindowSurface(g_sShow.pWindow);

	// Erase BG
	SDL_FillRect(
		g_sShow.pSurfWnd, NULL,
		SDL_MapRGB( g_sShow.pSurfWnd->format, SDL_fabs(SDL_sin(t))*0x7F, 0, 0 )
	);

	// Draw connection status
	gfxDrawConnState();

	// Draw hall
	gfxDrawHall();

	SDL_UpdateWindowSurface(g_sShow.pWindow);
	t += 0.001;
}

void gfxIdle(uv_idle_t *pIdle) {
	SDL_PollEvent(&g_sShow.sEvt);
	switch(g_sShow.sEvt.type) {
		case SDL_QUIT:
			uv_stop(g_sNetManager.pLoop);
			break;
	}
	gfxUpdate();
}

void gfxDrawText(const char *szTxt, SDL_Color *pColor, UWORD uwX, UWORD uwY) {
	SDL_Surface *pSurf;
	SDL_Rect sDstRect;

	pSurf = TTF_RenderText_Solid(g_sShow.pFont, szTxt, *pColor);
	if(!pSurf) {
		logError("RenderText: %s", TTF_GetError());
		return;
	}
	sDstRect.x = uwX;
	sDstRect.y = uwY;
	sDstRect.w = pSurf->w;
	sDstRect.h = pSurf->h;
	SDL_BlitSurface(pSurf, 0, g_sShow.pSurfWnd, &sDstRect);
	SDL_FreeSurface(pSurf);
}

void gfxDrawConnState(void) {
	SDL_Color sWhite = {0xFF, 0xFF, 0xFF};
	SDL_Color sGreen = {0, 0xFF, 0};
	SDL_Color sRed = {0xFF, 0, 0};

	// Connection to Hall
	gfxDrawText("Connection to hall: ", &sWhite, 0, 0);
	if(g_sShow.pClientHall->ubConnectState == CONNECTSTATE_OK)
		gfxDrawText("OK", &sGreen, 150, 0);
	else
		gfxDrawText("FAIL", &sRed, 150, 0);

	// Connection to Leader
	gfxDrawText("Connection to Leader: ", &sWhite, 0, 20);
	if(g_sShow.pClientLeader->ubConnectState == CONNECTSTATE_OK)
		gfxDrawText("OK", &sGreen, 150, 20);
	else
		gfxDrawText("FAIL", &sRed, 150, 20);

	// Hall
	// TODO: Connected client count
	// ...
	// TODO: Connected arms
	// ...
}

void gfxDrawHall(void) {
	UBYTE x,y;
	// Draw fields
	for(x = 0; x != 0; ++x) {
		for(y = 0; y != 0; ++y) {
			// TODO: Draw field with arm's color
			// ...
			// TODO: has field a platform?
			if(1) {
				// TODO: Draw platform
				// ...
				// TODO: Has platform a package?
				if(1) {
					// TODO: Draw package
					// ...
					// TODO: Show package destination - yellow
					// ...
				}
			}
		}
	}

	// TODO: Draw arms
	// ...
	// TODO: Draw arm route
}

#include "gfx.h"
#include <SDL.h>
#include "show.h"
#include "../common/log.h"

void gfxCreate(void) {
	SDL_Init(SDL_INIT_VIDEO);

	g_sShow.pWindow = SDL_CreateWindow(
		"Show", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_SHOWN
	);
	g_sShow.pSurfWnd = SDL_GetWindowSurface(g_sShow.pWindow);
	if(!g_sShow.pWindow)
		logError("Can't create SDL window!");

}

void gfxDestroy(void) {
	SDL_DestroyWindow(g_sShow.pWindow);
	SDL_Quit();
}

void gfxUpdate(void) {
	static double t = 0;
	g_sShow.pSurfWnd = SDL_GetWindowSurface(g_sShow.pWindow);
	SDL_FillRect(
		g_sShow.pSurfWnd, NULL,
		SDL_MapRGB( g_sShow.pSurfWnd->format, fabs(sin(t))*0xFF, 0, 0 )
	);
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

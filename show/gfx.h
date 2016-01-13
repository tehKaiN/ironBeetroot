#ifndef GUARD_SHOW_GFX_H
#define GUARD_SHOW_GFX_H

#include <uv.h>
#include "../common/types.h"

void gfxCreate(void);

void gfxDestroy(void);

void gfxUpdate(void);

void gfxIdle(
	IN uv_idle_t *pIdle
);

void gfxDrawConnState(void);

void gfxDrawHall(void);

#endif // GUARD_SHOW_GFX_H


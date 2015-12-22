#ifndef CONFIG_H
#define CONFIG_H

#include "../common/types.h"

typedef struct _tConfig{
	UBYTE ubBacklogLength; /// UV: backlog
  UWORD uwServerTimeout; /// Timeout for clients in seconds
} tConfig;

extern tConfig g_sConfig;

void configLoad(void);

#endif // CONFIG_H


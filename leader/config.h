#ifndef GUARD_LEADER_CONFIG_H
#define GUARD_LEADER_CONFIG_H

#include "../common/types.h"

typedef struct _tConfig{
	                    // Hall connection info
	char szHallIP[16]; /// IP
	UWORD uwHallPort;  /// Port
	                    // Arm server info
	UWORD uwArmPort;   /// Listen port
} tConfig;

void configLoad(void);

extern tConfig g_sConfig;

#endif // GUARD_LEADER_CONFIG_H


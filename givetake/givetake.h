#ifndef GUARD_GIVETAKE_GIVETAKE_H
#define GUARD_GIVETAKE_GIVETAKE_H

#define NEWPACKAGE_STATE_UNKNOWN 0
#define NEWPACKAGE_STATE_SENDING 1
#define NEWPACKAGE_STATE_RESEND  2
#define NEWPACKAGE_STATE_SENT    3

typedef struct _tNadaniePackage{
	UBYTE ubState;       /// See NEWPACKAGE_STATE_* macros
	UBYTE ubPlatformDst; /// Dest platform id
} tNadaniePackage;

typedef struct _tNadanie{
	tNetClient *pClient;      /// Connection to hall
	UBYTE ubPlatformSend;     /// Platform for sending packages
	UBYTE ubPlatformRecv;     /// Platform for receiving packages
	UBYTE ubPlatformSendBusy; /// Is platform for sending busy
	UBYTE ubDestinationCount; /// Total count of available destinations
	UBYTE *pDestinations;     /// Array of available destinations
	UBYTE ubReady;            /// Identified and ready to poll Hall for packages
	uv_timer_t sTimer;        /// Update timer
	tNadaniePackage sPackage; /// Package pending to be sent
	uv_mutex_t sPackageMutex; /// Mutex for updating sPackage
} tNadanie;

void nadanieCreate(void);

void nadanieDestroy(void);

void nadanieProcessPacket(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void nadanieOnConnect(
	IN tNetClient *pClient
);

void nadanieUpdate(
	IN uv_timer_t* handle
);

UBYTE nadanieRandomDestination(void);

tNadanie g_sNadanie;

#endif // GUARD_GIVETAKE_NADANIE_H

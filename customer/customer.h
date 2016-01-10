#ifndef GUARD_CUSTOMER_CUSTOMER_H
#define GUARD_CUSTOMER_CUSTOMER_H

#define NEWPACKAGE_STATE_UNKNOWN 0
#define NEWPACKAGE_STATE_SENDING 1
#define NEWPACKAGE_STATE_RESEND  2
#define NEWPACKAGE_STATE_SENT    3

#define READY_ID 1
#define READY_LIST 2

typedef struct _tCustomerPackage{
	UBYTE ubState;       /// See NEWPACKAGE_STATE_* macros
	UBYTE ubPlatformDst; /// Dest platform id
} tCustomerPackage;

typedef struct _tCustomer{
	tNetClient *pClient;       /// Connection to hall
	UBYTE ubReady;             /// Identified and ready to poll Hall for packages
	uv_timer_t sPackageTimer;  /// Package update timer
	uv_timer_t sPlatformTimer; /// Platform list update timer
	                            // Package fields:
	tCustomerPackage sPackage;  /// Package pending to be sent
	uv_mutex_t sPackageMutex;  /// Mutex for updating sPackage
	                            // Destination fields:
	uv_mutex_t sDestMutex;     /// for ubDestCount & pDestList
	UBYTE ubDestCount;         /// Total count of available destinations
	UBYTE *pDestList;          /// Array of available destinations
} tCustomer;

void customerCreate(void);

void customerDestroy(void);

void customerProcessPacket(
	IN tNetClientServer *pClientServer,
	IN tNetConn *pClient,
	IN tPacket *pPacket
);

void customerOnConnect(
	IN tNetClient *pClient
);

void customerPackageUpdate (
	IN uv_timer_t* handle
);

void customerPlatformUpdate (
	IN uv_timer_t* handle
);

UBYTE customerRandomDestination(
	OUT UBYTE *pDest
);

tCustomer g_sCustomer;

#endif // GUARD_CUSTOMER_CUSTOMER_H

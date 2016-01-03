#ifndef GUARD_COMMON_NET_NET_H
#define GUARD_COMMON_NET_NET_H

#include <uv.h>
#include <time.h>
#include "../types.h"
#include "../list.h"
#include "../packet.h"

#define NET_UNKNOWN 0
#define NET_CLIENT 1
#define NET_SERVER 2

#define CONNECTSTATE_LOST 0
#define CONNECTSTATE_NEW 1
#define CONNECTSTATE_OK 2
#define CONNECTSTATE_RETRY 3

struct _tNetServer;
struct _tNetConn;

typedef void fnPacketProcess(
	IN struct _tNetServer *pServer,
	IN struct _tNetConn *pClient,
	IN tPacket *pPacket
);

typedef struct _tNetManager{
	UBYTE ubIsRunning;        /// 1: running, 0: not
	UBYTE ubClientCount;
	UBYTE ubServerCount;
	uv_loop_t *pLoop;         /// UV Loop
	tList *pClientServerList; /// Client/Server list
} tNetManager;

typedef struct _tNetConn{
	uv_tcp_t *pTCP;                          /// UV TCP handle - data field has backlink
	UBYTE ubType;                            /// Client type
	UBYTE ubActive;                          /// 1: connected, 0: not
	time_t llLastPacketTime;                 /// Last received packet timestamp
	struct _tNetClientServer *pClientServer; /// Backlink to associated client/server
} tNetConn;

typedef struct _tNetClient{
	uv_connect_t sConn;           /// UV Connection handle
	struct sockaddr_in sSrvAddr; /// Socket config for (re)connecting
	char szIP[12];                /// Connection IP addr
	UWORD uwPort;                 /// Connection port
	UBYTE ubConnectState;         /// See CONNECTSTATE_* macros
} tNetClient;

typedef struct _tNetServer{
	UBYTE ubMaxClients;    /// Max client count
	UWORD uwTimeout;       /// Max packet time distance
	tNetConn *pClients;    /// Array of connected clients
	uv_mutex_t sListMutex; /// Mutex for client list modification
} tNetServer;

typedef struct _tNetClientServer{
  union {                         /// tNetClient & tNetServer are castable to tNetClientServer
		tNetClient sClient;
		tNetServer sServer;
  };
	UBYTE ubType;                    /// NET_UNKNOWN | NET_CLIENT | NET_SERVER
	uv_tcp_t sTCP;                   /// UV TCP handle - data field has backlink
	fnPacketProcess *pPacketProcess; /// Incoming packet process callback
} tNetClientServer;

extern tNetManager g_sNetManager;

void netCreate(void);
void netRun(void);
void netDestroy(void);

void netOnRead(
	IN uv_stream_t *pClientStream,
	IN ssize_t lDataLength,
	IN const uv_buf_t *pBuf
);

void netDestroyClientServer(
	IN tNetClientServer *pClientServer
);

UBYTE netGetPacket(
	IN tPacket *pPacket,
	IN const uv_buf_t *pBuf,
	IN ULONG *pBufPos,
	IN LONG lDataLength
);

void netUpdateConnTime(
	IN tNetConn *pConn
);

#endif // GUARD_COMMON_NET_NET_H

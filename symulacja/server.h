#ifndef SERVER_H
#define SERVER_H

#include <uv.h>
#include "../common/types.h"
#include "../common/packet.h"

typedef struct _tClient{
	uv_tcp_t *pTCP;
	UBYTE ubType;
	UBYTE ubActive;
	time_t llLastPacketTime;
} tClient;

typedef void fnPacketProcess(
	IN tClient *pClient,
	IN tPacket *pPacket
);

typedef struct {
	fnPacketProcess *pPacketProcess; /// Protocol process function
	uv_loop_t *pLoop;                /// UV server loop
	tClient *pClientList;            /// List of connected clients
	UBYTE ubMaxClients;              /// Max client count
	uv_mutex_t sMutex;               /// Mutex for client list modification
} tServer;

extern tServer g_sServer;

void serverRun(
	IN UBYTE ubMaxClients,
	IN UWORD uwPort,
	IN fnPacketProcess *pPacketProcess
);

void serverOnConnect(
	IN uv_stream_t *pServerStream,
	IN LONG lStatus
);

void serverOnRead(
	IN uv_stream_t *pClientStream,
	IN ssize_t ulLength,
	IN const uv_buf_t *pBuf
);

void serverAllocFn(
	IN uv_handle_t *pHandle,
	IN size_t uiSuggestedSize,
	OUT uv_buf_t *pBuf
);

/**
 * Returns idx of free client slot
 * If not found, returns 0
 */
UBYTE serverGetFreeClientIdx(
	OUT UBYTE *pIdx
);

/**
 * Accepts new client and generates its TCP handle
 */
tClient *serverAddClient(
	IN uv_stream_t *pServerStream
);

/**
 * Returns pointer to active client with given uv_tcp_t
 * If client is not found, returns 0
 */
tClient *serverGetClientByTCP(
	IN uv_tcp_t *pTCP
);

void serverRmClient(
	IN tClient *pClient;
);

void serverRmClientByIdx(
	IN UBYTE ubIdx
);

void serverRmAll(void);

void serverUpdateClientTime(
	IN tClient *pClient
);

void serverKeepAlive(void);

UBYTE serverGetPacket(
	INOUT tPacket *pPacket,
	IN const uv_buf_t *pBuf,
	INOUT ULONG *pBufPos,
	IN LONG lDataLength
);

void serverProcessPacket(
	IN tClient *pClient,
	IN tPacket *pPacket
);

#endif // SERVER_H


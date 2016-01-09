#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
#include "givetake.h"
#include "config.h"
#include "process.h"

int main(void) {
	logCreate("givetake.log");
	memCreate();
	netCreate();
	nadanieCreate();

	g_sNadanie.pClient = netClientCreate(
		"127.0.0.1", 888,
		nadanieOnConnect, nadanieProcessPacket
	);
	netRun();

	nadanieDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void nadanieCreate(void) {
	g_sNadanie.ubReady = 0;
	g_sNadanie.ubPlatformRecv = 0;
	g_sNadanie.ubPlatformSend = 0;
  uv_mutex_init(&g_sNadanie.sPackageMutex);
	uv_timer_init(g_sNetManager.pLoop, &g_sNadanie.sTimer);
	uv_timer_start(&g_sNadanie.sTimer, nadanieUpdate, 1000, 1000);
}

void nadanieDestroy(void) {
	uv_mutex_destroy(&g_sNadanie.sPackageMutex);
}

void nadanieOnConnect(tNetClient *pClient) {
	tPacket sPacket;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_pClientTypes[CLIENT_TYPE_GIVETAKE], CLIENT_TYPE_GIVETAKE
	);

	packetMakeSetType(&sPacket, CLIENT_TYPE_GIVETAKE);
	netSend(&pClient->sSrvConn, &sPacket, netReadAfterWrite);
}

void nadanieProcessPacket(
	tNetClientServer *pServer, tNetConn *pClient, tPacket *pPacket
) {
	switch(pPacket->sHead.ubType) {
		case PACKET_R_SETTYPE: {
			tPacketSetTypeResponse *pResponse;
			tPacketHead sRequest;

			pResponse = (tPacketSetTypeResponse*)pPacket;
			if(!pResponse->ubIsOk) {
				logError("Identification failed");
				// TODO: Close connection
				// ...
				return;
			}
			logSuccess(
				"Successfully identified as %s",
				g_pClientTypes[CLIENT_TYPE_GIVETAKE]
			);

			// TODO(#5): Heartbeat timer
			// ...

			// Get platform info
			packetMakeEmpty(&sRequest, PACKET_GETPLATFORMINFO);
			logWrite("Sending platform info request");
			netSend(
				&g_sNadanie.pClient->sSrvConn, (tPacket*)&sRequest, netReadAfterWrite
			);
		}break;
		case PACKET_R_GETPLATFORMINFO: {
			tPacketPlatformInfo *pInfo;

			logWrite("Got platform info request");
			pInfo = (tPacketPlatformInfo *)pPacket;
			g_sNadanie.ubPlatformRecv = pInfo->ubPlatformRecv;
			g_sNadanie.ubPlatformSend = pInfo->ubPlatformSend;

			// Get ready to send new package
			g_sNadanie.sPackage.ubState = NEWPACKAGE_STATE_SENT;
			g_sNadanie.ubReady = 0;
			} break;
		case PACKET_R_UPDATEPLATFORMS: {
			tPacketUpdatePlatformsResponse *pResponse;

			pResponse = (tPacketUpdatePlatformsResponse *)pPacket;

			if(pResponse->ubPlaced) {
				uv_mutex_lock(&g_sNadanie.sPackageMutex);
				g_sNadanie.sPackage.ubState = NEWPACKAGE_STATE_SENT;
				uv_mutex_unlock(&g_sNadanie.sPackageMutex);

				g_sNadanie.ubPlatformSendBusy = 0;
				logSuccess("Successfully placed package");
			}
			else {
				if(!g_sNadanie.ubPlatformSendBusy) {
					logWrite("Source platform is busy");
					g_sNadanie.ubPlatformSendBusy = 1;
				}
				uv_mutex_lock(&g_sNadanie.sPackageMutex);
				g_sNadanie.sPackage.ubState = NEWPACKAGE_STATE_RESEND;
				uv_mutex_unlock(&g_sNadanie.sPackageMutex);
			}
			if(pResponse->ubGrabbed)
				logWrite("Grabbed package %hu", pResponse->ubRecvPackageId);
			} break;
		default:
			logWarning("Unknown packet: %hu", pPacket->sHead.ubType);
			logBinary(pPacket, pPacket->sHead.ubPacketLength);
	}
}

void nadanieUpdate(uv_timer_t *pTimer) {
	tNadaniePackage *pPackage;

	if(g_sNadanie.pClient->ubConnectState != CONNECTSTATE_OK)
		return;
	if(!g_sNadanie.ubReady)
		return;
	logWrite("nadanieUpdate");

	pPackage = &g_sNadanie.sPackage;
  switch(pPackage->ubState) {
		case NEWPACKAGE_STATE_UNKNOWN:
			logError("Unknown package state");

			// Assume there is no package - generate new
			uv_mutex_lock(&g_sNadanie.sPackageMutex);
			pPackage->ubState = NEWPACKAGE_STATE_SENT;
			uv_mutex_unlock(&g_sNadanie.sPackageMutex);
			break;
		case NEWPACKAGE_STATE_SENT: {
			tPacketUpdatePlatforms sPacket;

			// Generate random route
			sPacket.sHead.ubType = PACKET_UPDATEPLATFORMS;
			sPacket.sHead.ubPacketLength = sizeof(tPacketUpdatePlatforms);
			sPacket.ubPlatformDst = nadanieRandomDestination();

			uv_mutex_lock(&g_sNadanie.sPackageMutex);
			pPackage->ubPlatformDst = sPacket.ubPlatformDst;
			pPackage->ubState = NEWPACKAGE_STATE_SENDING;
			uv_mutex_unlock(&g_sNadanie.sPackageMutex);

			// Send package
			netSend(
				&g_sNadanie.pClient->sSrvConn,
				(tPacket*)&sPacket,
				netReadAfterWrite
			);
			} break;
		case NEWPACKAGE_STATE_RESEND: {
			tPacketUpdatePlatforms sPacket;

			sPacket.sHead.ubType = PACKET_UPDATEPLATFORMS;
			sPacket.sHead.ubPacketLength = sizeof(tPacketUpdatePlatforms);

			uv_mutex_lock(&g_sNadanie.sPackageMutex);
			sPacket.ubPlatformDst = g_sNadanie.sPackage.ubPlatformDst;
			uv_mutex_unlock(&g_sNadanie.sPackageMutex);

			// Send package
			netSend(
				&g_sNadanie.pClient->sSrvConn,
				(tPacket*)&sPacket,
				netReadAfterWrite
			);
			} break;
  }
}

UBYTE nadanieRandomDestination(void) {
	// TODO(#9): Get random destination platform
	return 8;
}


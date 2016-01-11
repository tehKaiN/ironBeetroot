#include "../common/mem.h"
#include "../common/log.h"
#include "../common/net/net.h"
#include "../common/net/client.h"
#include "../common/packet.h"
#include "customer.h"
#include "config.h"
#include "process.h"

int main(void) {
	logCreate("customer.log");
	memCreate("customer_mem.log");
	netCreate();
	customerCreate();

	g_sCustomer.pClient = netClientCreate(
		"127.0.0.1", 888,
		customerOnConnect, processPacket
	);
	netRun();

	customerDestroy();
	netDestroy();
	memDestroy();
	logDestroy();
	return 0;
}

void customerCreate(void) {
	g_sCustomer.ubReady = 0;

	// Init mutex
  uv_mutex_init(&g_sCustomer.sPackageMutex);
  uv_mutex_init(&g_sCustomer.sDestMutex);

  // Init timers
	uv_timer_init(g_sNetManager.pLoop, &g_sCustomer.sPackageTimer);
	uv_timer_init(g_sNetManager.pLoop, &g_sCustomer.sPlatformTimer);
	uv_timer_start(&g_sCustomer.sPackageTimer, customerPackageUpdate, 1000, 5000);
	uv_timer_start(&g_sCustomer.sPlatformTimer, customerPlatformUpdate, 500, 500);
	// TODO(#3): Heartbeat timer
	// ...
}

void customerDestroy(void) {
	uv_mutex_destroy(&g_sCustomer.sPackageMutex);
  uv_mutex_destroy(&g_sCustomer.sDestMutex);
}

void customerOnConnect(tNetClient *pClient) {
	tPacketSetType sPacket;

	g_sCustomer.ubReady = 0;

	// Send ID packet
	logWrite(
		"Sending client type: '%s\' (%u)",
		g_pClientTypes[CLIENT_TYPE_CUSTOMER], CLIENT_TYPE_CUSTOMER
	);
	packetMakeSetType(&sPacket, CLIENT_TYPE_CUSTOMER);
	netSend(&pClient->sSrvConn, (tPacket*)&sPacket, netReadOnWrite);
}

void customerPackageUpdate(uv_timer_t *pTimer) {
	tCustomerPackage *pPackage;

	if(!g_sCustomer.pClient)
		return;
	if(g_sCustomer.pClient->ubConnectState != CONNECTSTATE_OK)
		return;
	if(!(g_sCustomer.ubReady & READY_LIST))
		return;

	pPackage = &g_sCustomer.sPackage;
  switch(pPackage->ubState) {
		case NEWPACKAGE_STATE_UNKNOWN:
			logError("Unknown package state");

			// Assume there is no package - generate new
			uv_mutex_lock(&g_sCustomer.sPackageMutex);
			pPackage->ubState = NEWPACKAGE_STATE_SENT;
			uv_mutex_unlock(&g_sCustomer.sPackageMutex);
			break;
		case NEWPACKAGE_STATE_SENT: {
			tPacketUpdatePlatforms sPacket;

			// Generate random route
			sPacket.sHead.ubType = PACKET_UPDATEPLATFORMS;
			sPacket.sHead.ubPacketLength = sizeof(tPacketUpdatePlatforms);
			if(!customerRandomDestination(&sPacket.ubPlatformDst)) {
				logWarning("No available destinations");
				return;
			}

			uv_mutex_lock(&g_sCustomer.sPackageMutex);
			pPackage->ubPlatformDst = sPacket.ubPlatformDst;
			pPackage->ubState = NEWPACKAGE_STATE_SENDING;
			uv_mutex_unlock(&g_sCustomer.sPackageMutex);

			// Send package
			netSend(&g_sCustomer.pClient->sSrvConn, (tPacket*)&sPacket, netNopOnWrite);
			} break;
		case NEWPACKAGE_STATE_RESEND: {
			tPacketUpdatePlatforms sPacket;

			sPacket.sHead.ubType = PACKET_UPDATEPLATFORMS;
			sPacket.sHead.ubPacketLength = sizeof(tPacketUpdatePlatforms);

			uv_mutex_lock(&g_sCustomer.sPackageMutex);
			sPacket.ubPlatformDst = g_sCustomer.sPackage.ubPlatformDst;
			uv_mutex_unlock(&g_sCustomer.sPackageMutex);

			// Send package
			netSend(&g_sCustomer.pClient->sSrvConn, (tPacket*)&sPacket, netNopOnWrite);
			} break;
  }
}

void customerPlatformUpdate(uv_timer_t *pTimer) {
	tPacketHead sRequest;

	if(!g_sCustomer.pClient)
		return;
	if(!(g_sCustomer.ubReady & READY_ID))
		return;

	packetPrepare(
		(tPacket*)&sRequest, PACKET_GETPLATFORMINFO, sizeof(tPacketHead)
	);
	netSend(&g_sCustomer.pClient->sSrvConn, (tPacket*)&sRequest, netNopOnWrite);
}

UBYTE customerRandomDestination(UBYTE *pDest) {
	uv_mutex_lock(&g_sCustomer.sDestMutex);
	if(!g_sCustomer.ubDestCount) {
		uv_mutex_unlock(&g_sCustomer.sDestMutex);
		return 0;
	}

	*pDest = g_sCustomer.pDestList[rand() % g_sCustomer.ubDestCount];
	uv_mutex_unlock(&g_sCustomer.sDestMutex);
	return 1;
}


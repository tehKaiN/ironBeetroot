#include "process.h"
#include "client.h"
#include "../common/packet.h"

void processClient(void) {
	tPacket sPacket;

	// Send ID packet
	packetMakeHello(&sPacket, CLIENT_TYPE_NADANIE);
	clientSend(&sPacket);

	// do stuff
}

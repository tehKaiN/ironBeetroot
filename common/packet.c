#include <stdlib.h>
#include "packet.h"
#include "log.h"

const char *g_pClientTypes[CLIENT_TYPES] = {
	"Unknown",
	"Arm",
	"Leader",
	"Customer",
	"Show",
	"Hall"
};

void packetPrepare(tPacket *pPacket, UBYTE ubType, UBYTE ubSize) {
	memset(pPacket, 0, ubSize);
	pPacket->sHead.ubPacketLength = ubSize;
	pPacket->sHead.ubType  = ubType;
	pPacket->sHead.uwRand = rand() & 0xFFFF;
}

void packetMakeSetType(tPacketSetType *pPacket, UBYTE ubClientType) {
	packetPrepare((tPacket*)pPacket, PACKET_SETTYPE, sizeof(tPacketSetType));
	pPacket->ubClientType = ubClientType;
}

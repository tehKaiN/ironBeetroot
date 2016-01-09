#include <stdlib.h>
#include "packet.h"

const char *g_pClientTypes[CLIENT_TYPES] = {
	"Unknown",
	"Arm",
	"Leader",
	"GiveTake",
	"Show",
	"Hall"
};

void packetMakeHead(tPacketHead *pHead, UBYTE ubType, UBYTE ubSize) {
	pHead->ubPacketLength = ubSize;
	pHead->ubType  = ubType;
	pHead->uwRand = rand() & 0xFFFF;
}

void packetMakeSetType(tPacket *pPacket, UBYTE ubClientType) {
	tPacketSetType *pSetType;

	pSetType = (tPacketSetType *)pPacket;
	pSetType->sHead.ubPacketLength = sizeof(tPacketSetType);
	pSetType->sHead.ubType = PACKET_SETTYPE;

	pSetType->ubClientType = ubClientType;
}

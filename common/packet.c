#include "packet.h"

void packetMakeEmpty(tPacket *pPacket, UBYTE ubType) {
	pPacket->sHead.ubPacketLength = sizeof(tPacketHead);
	pPacket->sHead.ubType  = ubType;
}

void packetMakeHello(tPacket *pPacket, UBYTE ubClientType) {
	pPacket->sHead.ubPacketLength = sizeof(tPacketHead) + sizeof(tPacketHello);
	pPacket->sHead.ubType = PACKET_HELLO;

	pPacket->sHello.ubClientType = ubClientType;
}

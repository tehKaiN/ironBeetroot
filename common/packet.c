#include "packet.h"

const char *g_szClientTypes[CLIENT_TYPES] = {
	"Unknown",
	"Ramie",
	"Logistyka",
	"Nadanie",
	"Odbior",
	"Wizualizacja",
	"Symulacja"
};

void packetMakeEmpty(tPacket *pPacket, UBYTE ubType) {
	pPacket->sHead.ubPacketLength = sizeof(tPacketHead);
	pPacket->sHead.ubType  = ubType;
}

void packetMakeHello(tPacket *pPacket, UBYTE ubClientType) {
	pPacket->sHead.ubPacketLength = sizeof(tPacketHead) + sizeof(tPacketHello);
	pPacket->sHead.ubType = PACKET_HELLO;

	pPacket->sHello.ubClientType = ubClientType;
}

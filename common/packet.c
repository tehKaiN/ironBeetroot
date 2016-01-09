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

void packetMakeSetType(tPacket *pPacket, UBYTE ubClientType) {
	tPacketSetType *pSetType;

	pSetType = (tPacketSetType *)pPacket;
	pSetType->sHead.ubPacketLength = sizeof(tPacketHead) + sizeof(tPacketSetType);
	pSetType->sHead.ubType = PACKET_SETTYPE;

	pSetType->ubClientType = ubClientType;
}

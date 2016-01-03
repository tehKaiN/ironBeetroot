#include "platform.h"
#include "symulacja.h"

void platformInit(UBYTE ubIdx, UBYTE ubFieldX, UBYTE ubFieldY, UBYTE ubType) {
	tPlatform *pPlatform;

	pPlatform = &g_sSim.pPlatforms[ubIdx];

	// Set flag on field
	g_sSim.pFields[ubFieldX][ubFieldY] |= FIELD_PLATFORM;

	// Fill platform struct
	pPlatform->pPackage = 0;
	pPlatform->szName = 0;
	pPlatform->ubFieldX = ubFieldX;
	pPlatform->ubFieldY = ubFieldY;
	pPlatform->ubId = ubIdx;
	pPlatform->ubType = ubType;
}

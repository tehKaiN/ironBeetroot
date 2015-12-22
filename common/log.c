/**
 * Logging functions
 */

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "log.h"

#if __WIN32
/**
 * Win32API implementation
 * Uses SetConsoleTextAttribute function
 */
void logSetColor(UBYTE ubColor) {
	UWORD pColorCodes[] = {
		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN, FOREGROUND_INTENSITY
	};
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), pColorCodes[ubColor]);
}
#else
#include "../common/types.h"
/**
 * Linux implementation
 * Uses ANSI escape codes
 */
void logSetColor(UBYTE ubColor) {
	char *pColorCodes[] = {"\e[0m", "\e[1;32m", "\e[1;33m", "\e[1;31m"}; // TODO: white
	printf("%s", pColorCodes[ubColor]);
}
#endif // __WIN32

void logPrintDate(void) {
	char szDateBfr[LOG_DATE_BUFFER_SIZE];
	time_t lTimeStamp;
	struct tm *pTimeInfo;

	time(&lTimeStamp);
	pTimeInfo = localtime(&lTimeStamp);

  strftime(szDateBfr, LOG_DATE_BUFFER_SIZE, "[%H:%M:%S]", pTimeInfo);
  printf("%s ", szDateBfr);
}

void logWriteColor(UBYTE ubColor, char *szFmt, ...) {
	va_list vaArgs;

	logSetColor(ubColor);

	logPrintDate();
	va_start(vaArgs, szFmt);
	vprintf(szFmt, vaArgs);
	va_end(vaArgs);
}

void logBinary(void *pData, UWORD uwSize) {
	UWORD i;

	logSetColor(LOG_COLOR_WHITE);
	logPrintDate();
	printf("Bin(%u): ", uwSize);
	for(i = 0; i != uwSize; ++i) {
    printf("%0hX ", ((UBYTE*)pData)[i]);
	}
}

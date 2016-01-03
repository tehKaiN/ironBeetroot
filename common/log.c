/**
 * Logging functions
 */

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <uv.h>
#include "log.h"

#if __WIN32
/**
 * Win32API implementation
 * Uses SetConsoleTextAttribute function
 */
void logSetColor(UBYTE ubColor) {
	UWORD pColorCodes[] = {
		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,      // Grey
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,                  // Green
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, // Yellow
		FOREGROUND_RED | FOREGROUND_INTENSITY,                    // Red
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN, FOREGROUND_INTENSITY
	};
	SetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), pColorCodes[ubColor]
	);
}
#else
#include "../common/types.h"
/**
 * Linux implementation
 * Uses ANSI escape codes
 */
void logSetColor(UBYTE ubColor) {
	char *pColorCodes[] = {
		"\e[0m",    // Grey
		"\e[1;32m", // Green
		"\e[1;33m", // Yellow
		"\e[1;31m", // Red
		"\e[1;37m"  // White
	};
	printf("%s", pColorCodes[ubColor]);
}
#endif // __WIN32

void logCreate(char *szLogName) {
	uv_mutex_init(&g_sLogManager.sMutex);
	g_sLogManager.pFile = fopen(szLogName, "w");
	logSuccess("Log %s started", szLogName);
}

void logDestroy(void) {
	uv_mutex_destroy(&g_sLogManager.sMutex);
	fclose(g_sLogManager.pFile);
}

void logPrintDate(void) {
	char szDateBfr[LOG_DATE_BUFFER_SIZE];
	time_t lTimeStamp;
	struct tm *pTimeInfo;

	time(&lTimeStamp);
	pTimeInfo = localtime(&lTimeStamp);

  strftime(szDateBfr, LOG_DATE_BUFFER_SIZE, "[%H:%M:%S]", pTimeInfo);
  printf("%s", szDateBfr);
  fprintf(g_sLogManager.pFile, "%s", szDateBfr);
}

void logWriteColor(UBYTE ubColor, const char *szFnName, char *szFmt, ...) {
	va_list vaArgs;

	uv_mutex_lock(&g_sLogManager.sMutex);
	logSetColor(LOG_COLOR_GREY);
	logPrintDate();
	printf("[%s] ", szFnName);
	fprintf(g_sLogManager.pFile, "[%s] ", szFnName);
	logSetColor(ubColor);

	va_start(vaArgs, szFmt);
	vprintf(szFmt, vaArgs);
	vfprintf(g_sLogManager.pFile, szFmt, vaArgs);
	va_end(vaArgs);
	putc('\n', stdout);
	fputc('\n', g_sLogManager.pFile);
	uv_mutex_unlock(&g_sLogManager.sMutex);
}

void logBinary(void *pData, UWORD uwSize) {
	UWORD i;

	uv_mutex_lock(&g_sLogManager.sMutex);
	logPrintDate();
	logSetColor(LOG_COLOR_WHITE);
	printf("Bin(%u): ", uwSize);
	fprintf(g_sLogManager.pFile, "Bin(%u): ", uwSize);
	for(i = 0; i != uwSize; ++i) {
    printf("%0hX ", ((UBYTE*)pData)[i]);
    fprintf(g_sLogManager.pFile, "%0hX ", ((UBYTE*)pData)[i]);
	}
	putc('\n', stdout);
	fputc('\n', g_sLogManager.pFile);
	uv_mutex_unlock(&g_sLogManager.sMutex);
}

tLogManager g_sLogManager;

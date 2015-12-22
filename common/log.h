#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include "types.h"

#define LOG_COLOR_GREY 0
#define LOG_COLOR_GREEN 1
#define LOG_COLOR_YELLOW 2
#define LOG_COLOR_RED 3
#define LOG_COLOR_WHITE 4
#define LOG_DATE_BUFFER_SIZE 20

void logSetColor(
	IN UBYTE ubColor
);

void logPrintDate(void);

void logVPrint(
	IN char *szFmt,
	IN va_list vaArgs
);

void logWriteColor(
	IN UBYTE ubColor,
	IN char *szFmt,
	IN ...
);

void logBinary(
	IN void *pData,
	IN UWORD uwSize
);

#define logWrite(...) logWriteColor(LOG_COLOR_GREY, __VA_ARGS__)
#define logError(...) logWriteColor(LOG_COLOR_RED, __VA_ARGS__)
#define logWarning(...) logWriteColor(LOG_COLOR_YELLOW, __VA_ARGS__)
#define logSuccess(...) logWriteColor(LOG_COLOR_GREEN, __VA_ARGS__)

#endif // COMMON_LOG_H

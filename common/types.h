#include <stdint.h>

#define IN
#define OUT
#define INOUT

// Include windows
#include <windows.h>

// Undef windows types
#undef BYTE
#undef WORD
#undef LONG
#undef ULONG

// Include our types
#define UBYTE uint8_t
#define UWORD uint16_t
#define ULONG uint32_t

#define BYTE int8_t
#define WORD int16_t
#define LONG int32_t

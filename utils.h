#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include "types.h"

#ifdef _WIN32
#define PATH_SEPERATOR '\\'
#else
#define PATH_SEPERATOR '/'
#endif

#ifndef MAX_PATH
	#define MAX_PATH 255
#endif

#ifdef __cplusplus
extern "C" {
#endif

u64 getle64(const void* p);
u32 getle32(const void* p);
u32 getle16(const void* p);
u64 getbe64(const void* p);
u32 getbe32(const void* p);
u32 getbe16(const void* p);
void putle16(void* p, u16 n);
void putle32(void* p, u32 n);
void putle64(void* p, u64 n);
void putbe16(void* p, u16 n);
void putbe32(void* p, u32 n);
void putbe64(void* p, u64 n);

#ifdef __cplusplus
}
#endif

#endif // _UTILS_H_

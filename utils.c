#define _GNU_SOURCE
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

u64 getle64(const void* p)
{
    u8* p8 = (u8*)p;
	u64 n = p8[0];

	n |= (u64)p8[1]<<8;
	n |= (u64)p8[2]<<16;
	n |= (u64)p8[3]<<24;
	n |= (u64)p8[4]<<32;
	n |= (u64)p8[5]<<40;
	n |= (u64)p8[6]<<48;
	n |= (u64)p8[7]<<56;
	return n;
}

u64 getbe64(const void* p)
{
	u64 n = 0;
    u8* p8 = (u8*)p;

	n |= (u64)p8[0]<<56;
	n |= (u64)p8[1]<<48;
	n |= (u64)p8[2]<<40;
	n |= (u64)p8[3]<<32;
	n |= (u64)p8[4]<<24;
	n |= (u64)p8[5]<<16;
	n |= (u64)p8[6]<<8;
	n |= (u64)p8[7]<<0;
	return n;
}

u32 getle32(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<0) | (p8[1]<<8) | (p8[2]<<16) | (p8[3]<<24);
}

u32 getbe32(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<24) | (p8[1]<<16) | (p8[2]<<8) | (p8[3]<<0);
}

u32 getle16(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<0) | (p8[1]<<8);
}

u32 getbe16(const void* p)
{
    u8* p8 = (u8*)p;
	return (p8[0]<<8) | (p8[1]<<0);
}

#define ENDIAN_BIG 1
#define ENDIAN_LITTLE 2
u32 get_platform_endian()
{
    u32 test = 1;
    if(((char*)&test)[0] == 1)
        return ENDIAN_LITTLE;
    else
        return ENDIAN_BIG;
}

void reverse_copy(u8* dst, u8* src, u32 len)
{
    for(u32 i = 0; i < len; i++)
        dst[i] = src[len-1 - i];
}

// copy a primitive of len length, flipping endian if needed
void be_copy(void* out,  void* in, u32 len)
{
    u8* p8 = (u8*)out;
    u8* n8 = (u8*)in;
    if(get_platform_endian() == ENDIAN_BIG)
        memcpy(p8, n8, len);
    else
        reverse_copy(p8, n8, len);
}

// see be_copy
void le_copy(void* out, void* in, u32 len)
{
    u8* p8 = (u8*)out;
    u8* n8 = (u8*)in;
    if(get_platform_endian() == ENDIAN_LITTLE)
        memcpy(p8, n8, len);
    else
        reverse_copy(p8, n8, len);
}

void putbe16(void* p, u16 n)
{
    be_copy(p, &n, 2);
}

void putbe32(void* p, u32 n)
{
    be_copy(p, &n, 4);
}

void putbe64(void* p, u64 n)
{
    be_copy(p, &n, 8);
}

void putle16(void* p, u16 n)
{
    le_copy(p, &n, 2);
}

void putle32(void* p, u32 n)
{
    le_copy(p, &n, 4);
}

void putle64(void* p, u64 n)
{
    le_copy(p, &n, 8);
}
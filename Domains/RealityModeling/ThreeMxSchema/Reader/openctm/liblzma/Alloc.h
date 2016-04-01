/* Alloc.h -- Memory allocation functions
2008-03-13
Igor Pavlov
Public domain */

#ifndef __COMMON_ALLOC_H
#define __COMMON_ALLOC_H

#include <stddef.h>

#include "NameMangle.h"

void *MyAlloc(size_t size);
void MyFree(void *address);

#ifdef BENTLEY_WIN32

void SetLargePageSize();

#ifdef NO_BENTLEY_CHANGES
void *MidAlloc(size_t size);
#endif
void MidFree(void *address);
#ifdef NO_BENTLEY_CHANGES
void *BigAlloc(size_t size);
#endif
void BigFree(void *address);

#else

#define MidAlloc(size) MyAlloc(size)
#define MidFree(address) MyFree(address)
#define BigAlloc(size) MyAlloc(size)
#define BigFree(address) MyFree(address)

#endif

#endif

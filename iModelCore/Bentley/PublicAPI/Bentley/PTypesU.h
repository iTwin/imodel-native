/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/PTypesU.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#if defined (DATA_ALIGNMENT_FORCED)
typedef struct unalignedShort
    {
    char    sh [sizeof (short)];
    } UnalignedShort;

typedef struct unalignedLong
    {
    char    lo [sizeof (long)];
    } UnalignedLong;

typedef struct unalignedPointer
    {
    char    ptr [sizeof (void *)];
    } UnalignedPointer;

typedef struct unalignedDouble
    {
    char    db [sizeof (double)];
    } UnalignedDouble;

typedef struct unalignedInt64
    {
    char    db [sizeof (int64_t)];
    } UnalignedInt64;

#if defined (FULL_DOUBLE_ALIGNMENT)
typedef struct longAlignedDouble
    {
    long    dl [2];
    } LongAlignedDouble;

typedef struct longAlignedInt64
    {
    long    dl [2];
    } LongAlignedInt64;
#else
typedef double LongAlignedDouble;
typedef int64_t LongAlignedInt64;
#endif
#else
typedef short UnalignedShort;
typedef long  UnalignedLong;
typedef double UnalignedDouble, LongAlignedDouble;
typedef void *UnalignedPointer;
typedef int64_t UnalignedInt64, LongAlignedInt64;
#endif

typedef union
    {
    unsigned short ush;
    short       sh;
    UnalignedShort unalignedShort;
    }
Shorts;

typedef union
    {
    uint32_t    uLg;
    long        lg;
    UnalignedLong unalignedLong;
    }
Longs;

typedef union
    {
    void        *pVoid;
    UnalignedPointer uap;
    }
Pointers;

typedef struct
    {
    long         sd [2];
    } StackDouble, StackInt64;

typedef union
    {
    StackDouble  ld;
    double        d;
    } DoubleArg;

typedef union       ptypes_u
    {
    void            *pv;
    char            *pc;
    unsigned char   *puc;
    wchar_t         *pWideChar;
    uint16_t        *pUtf16Char;
    short           *ps;
    UnalignedShort  *pUnalignedShort;
    unsigned short  *pus;
    long            *pl;
    unsigned long   *pul;
    UnalignedLong   *pUnalignedLong;
    int             *pi;
    float           *pf;
    double          *pd;
    UnalignedDouble *pUnalignedDouble;
    LongAlignedDouble *pLongAlignedDouble;
    int64_t         *pi64;
    uint64_t        *pui64;
    UnalignedInt64  *pUnalignedInt64;
    LongAlignedInt64 *pLongAlignedInt64;
    void            **ppv;
    char            **ppc;
    short           **pps;
    unsigned short  **ppus;
    int             **ppi;
    long            **ppl;
    unsigned char   **ppuc;
    double          **ppd;
    uint64_t        **ppui64;
    }
Ptypes_u;


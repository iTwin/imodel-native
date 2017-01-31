/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/memfuncs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifndef NULL
#define NULL 0
#endif

typedef struct
    {
    void*  (*malloc)  (size_t size);
    void*  (*calloc)  (size_t numObjects, size_t objectSize);
    void*  (*realloc) (void* pMemory, size_t newSize);
    void   (*free)    (void* pMemory);
    } MemFuncs;

static MemFuncs *s_pMemFuncs = NULL;
static MemFuncs s_substituteMemFuncs = {NULL, NULL, NULL, NULL};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void BSIBaseGeom::SetMemoryFuncs
(
void*  (*MallocFunc)          (size_t size),
void*  (*CallocFunc)          (size_t numObjects, size_t objectSize),
void*  (*ReallocFunc)         (void* pMemory, size_t newSize),
void   (*FreeFunc)            (void* pMemory)
)
    {
    s_substituteMemFuncs.malloc     = MallocFunc;
    s_substituteMemFuncs.calloc     = CallocFunc;
    s_substituteMemFuncs.realloc    = ReallocFunc;
    s_substituteMemFuncs.free       = FreeFunc;
    s_pMemFuncs = &s_substituteMemFuncs;
    }

static int64_t s_numMalloc = 0;
static int64_t s_numFree   = 0;
static int64_t s_numCalloc = 0;
static int64_t s_numRealloc = 0;

int64_t BSIBaseGeom::GetNumMalloc () { return s_numMalloc;}
int64_t BSIBaseGeom::GetNumFree () { return s_numFree;}
int64_t BSIBaseGeom::GetNumCalloc () { return s_numCalloc;}
int64_t BSIBaseGeom::GetNumRealloc () { return s_numRealloc;}
int64_t BSIBaseGeom::GetAllocationDifference ()
    {
    int64_t diff = s_numMalloc + s_numCalloc - s_numFree;
    return diff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     *BSIBaseGeom::Malloc
(
size_t          dataSize
)
    {
    s_numMalloc++;
    if (s_pMemFuncs)
        return s_pMemFuncs->malloc (dataSize);

    return  malloc (dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     *BSIBaseGeom::Calloc
(
size_t          records,
size_t          recordSize
)
    {
    s_numCalloc++;
    if (s_pMemFuncs)
        return s_pMemFuncs->calloc (records, recordSize);
    return  calloc (records, recordSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     BSIBaseGeom::Free
(
void            *dataP
)
    {
    if (dataP != NULL)
        s_numFree++;

    if (s_pMemFuncs)
        s_pMemFuncs->free (dataP);
    else
        free (dataP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     *BSIBaseGeom::Realloc
(
void            *oldDataP,
size_t          newSize
)
    {
    if (NULL == oldDataP)
        s_numMalloc++;
    else
        s_numRealloc++;

    if (s_pMemFuncs)
        return s_pMemFuncs->realloc (oldDataP, newSize);
    return realloc (oldDataP, newSize);
    }


void     *BSIBaseGeom::Malloc (size_t dataSize, void *pHeapDescr)
    {
    return Malloc(dataSize);
    }
void     *BSIBaseGeom::Calloc (size_t records, size_t recordSize, void *pHeapDescr)
    {
    return Calloc (records, recordSize);
    }
void     BSIBaseGeom::Free (void *dataP, void *pHeapDescr)
    {
    return Free (dataP);
    }
void     *BSIBaseGeom::Realloc (void *oldDataP, size_t newSize, void *pHeapDescr)
    {
    return Realloc (oldDataP, newSize);
    }

int BSIBaseGeom::MallocAndCopy
(
double          **paramPP,
bvector<double> const &source
)
    {
    size_t n = source.size ();
    if (NULL == paramPP)
        {
        return (int)n;
        }
    else if (n == 0)
        {
        *paramPP = NULL;
        return 0;
        }
    else
        {
        *paramPP = (double*)BSIBaseGeom::Malloc (source.size () * sizeof (double));
        if (NULL != paramPP)
            {
            for (size_t i = 0; i < n; i++)
                (*paramPP)[i] = source[i];
            }
        return (int) n;
        }
    }

int BSIBaseGeom::MallocAndCopy
(
DPoint3d          **paramPP,
bvector<DPoint3d> const &source
)
    {
    size_t n = source.size ();
    if (NULL == paramPP)
        {
        return (int)n;
        }
    else if (n == 0)
        {
        *paramPP = NULL;
        return 0;
        }
    else
        {
        *paramPP = (DPoint3d*)BSIBaseGeom::Malloc (source.size () * sizeof (DPoint3d));
        if (NULL != paramPP)
            {
            for (size_t i = 0; i < n; i++)
                (*paramPP)[i] = source[i];
            }
        return (int) n;
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE

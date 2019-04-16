/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef __cplusplus

#if defined (INCLUDE_FROM_MDLDOC)
    typedef unsigned int size_t;
#else
//#include <stdio.h>
#endif

#include <Geom/msgeomstructs_typedefs.h>


class GEOMDLLIMPEXP BSIBaseGeom
{
private:
    BSIBaseGeom () {}
public:
//!
//! @description Install pointers to replacements for C runtime memory management functions.
//! @remarks Call this function once at the start of your session to use application-specific VU memory management.
//! @param [in] newMalloc  replacement for malloc
//! @param [in] newCalloc  replacement for calloc
//! @param [in] newRealloc  replacement for realloc
//! @param [in] newFree  replacement for free
//! @group "BSIBaseGeom Memory Management"
//!
static void SetMemoryFuncs
(
void*  (*MallocFunc)(size_t size),
void*  (*CallocFunc)(size_t numObjects, size_t objectSize),
void*  (*ReAlloc)   (void* pMemory, size_t newSize),
void   (*FreeFunc)  (void* pMemory));

//!
//! @description Allocate uninitialized bytes, by C runtime malloc or substitute function.
//! @param [in] dataSize  bytes to allocate
//! @return pointer to newly allocated memory or NULL if insufficient memory available
//! @group "BSIBaseGeom Memory Management"
//!
static void     *Malloc (size_t dataSize);

//!
//! @description Allocate zero-initialized bytes, by C runtime calloc or substitute function.
//! @param [in] records  number of records in buffer
//! @param [in] recordSize  bytes per record
//! @return pointer to newly allocated memory or NULL if insufficient memory available
//! @group "BSIBaseGeom Memory Management"
//!
static void     *Calloc
(
size_t          records,
size_t          recordSize
);

//!
//! @description Free memory previously allocated by
//!            ~mBSIBaseGeom::Malloc,
//!            ~mmBSIBaseGeom::Calloc or
//!            ~mmBSIBaseGeom::Realloc.
//! @param [in] dataP  pointer to memory to return to heap.
//! @group "BSIBaseGeom Memory Management"
//!
static void     Free
(
void            *dataP
);

//!
//! @description Resize memory previously allocated by
//!                    ~mBSIBaseGeom::Malloc,
//!                    ~mmBSIBaseGeom::Calloc or
//!                    ~mmBSIBaseGeom::Realloc.
//! @param [in] oldDataP  prior allocation
//! @param [in] newSize  bytes required in reallocated block
//! @return pointer to newly allocated memory or NULL if insufficient memory available
//! @group "BSIBaseGeom Memory Management"
//!
static void     *Realloc
(
void            *oldDataP,
size_t          newSize
);


// Stub functions for old code calling with a heap descriptor ..
static void     *Malloc (size_t dataSize, void *pHeapDescr);
static void     *Calloc (size_t records, size_t recordSize, void *pHeapDescr);
static void      Free   (void *dataP, void *pHeapDescr);
static void     *Realloc (void *oldDataP, size_t newSize, void *pHeapDescr);

//! Inquire the total number of Malloc  calls so far.
static int64_t GetNumMalloc ();
//! Inquire the total number of Free    calls so far.
static int64_t GetNumFree ();
//! Inquire the total number of Calloc  calls so far.
static int64_t GetNumCalloc ();
//! Inquire the total number of Realloc calls so far.
static int64_t GetNumRealloc ();
//! Inquire the difference of GetNumMalloc() + GetNumCalloc () - GetNumFree ();
static int64_t GetAllocationDifference ();

//! Copy bvector<double> to heap
//! Return number of doubles in source (even if paramPP is NULL and copy was skipped !!)
static int MallocAndCopy
(
double          **paramPP,
bvector<double> const &source
);

//! Copy bvector<double> to heap
//! Return number of doubles in source (even if paramPP is NULL and copy was skipped !!)
static int MallocAndCopy
(
DPoint3d    **paramPP,
bvector<DPoint3d> const &source
);


};
#endif
END_BENTLEY_GEOMETRY_NAMESPACE

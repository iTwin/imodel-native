/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#if defined (INCLUDE_FROM_MDLDOC)
    typedef unsigned int size_t;
#else
//#include <stdio.h>
#endif

/** @description Callback signature for application-specific malloc.
    @param size IN number of bytes to allocate
    @return pointer to newly allocated memory or NULL if insufficient memory available
    @group "VU Memory Management"
    @see vumemfuncs_setFuncs */
typedef void*  (*VuMallocFunc)  (size_t size);
/** @description Callback signature for application-specific calloc.
    @param numObjects IN number of objects to allocate
    @param objectSize IN number of bytes per object
    @return pointer to newly allocated memory or NULL if insufficient memory available
    @group "VU Memory Management"
    @see vumemfuncs_setFuncs */
typedef void*  (*VuCallocFunc)  (size_t numObjects, size_t objectSize);
/** @description Callback signature for application-specific realloc.
    @param pMemory IN pointer to buffer
    @param newSize IN number of bytes to allocate in new buffer
    @return pointer to newly allocated memory or NULL if insufficient memory available
    @group "VU Memory Management"
    @see vumemfuncs_setFuncs */
typedef void*  (*VuReallocFunc) (void* pMemory, size_t newSize);
/** @description Callback signature for application-specific free.
    @param pMemory IN pointer to buffer
    @group "VU Memory Management"
    @see vumemfuncs_setFuncs */
typedef void   (*VuFreeFunc) (void* pMemory);
END_BENTLEY_GEOMETRY_NAMESPACE

#include <Vu/capi/vumemfuncs_capi.h>


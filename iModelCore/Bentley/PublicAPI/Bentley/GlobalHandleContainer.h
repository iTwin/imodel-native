/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/GlobalHandleContainer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "Bentley.h"
#include <vector>
#include <queue>

BEGIN_BENTLEY_NAMESPACE


/*=================================================================================**//**
* Associates a pointer with a unique 32 bit handle.
*
* @remarks In cases where it is necessary to provide a unique 32-bit value
* it has been customary to provide a pointer.  This approach is not valid for 64-bit
* processes.  GlobalHandleContainer provides an approach that works equally well for
* 32-bit and 64-bit processes.
* @remarks The handles provide some protection against use of a handle that has
* already been freed even if the handle has been reallocated. Each handle contains
* an index and a counter of how often the index has been freed.  If the count in the
* handle does not match the count for the index slot then the handle is invalid.
* However, the count is held in 8 bits. It is possible that a handle that appears
* to be unchanged has actually cycled. This technique aids in catching bugs but is
* not reliable to be a crucial part of a design.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct           GlobalHandleContainer
    {
private:
    GlobalHandleContainer () {}

public:
    enum ReservedHandles
        {
        InvalidHandle   = 0
        };

    BENTLEYDLL_EXPORT static bool         IsHandleValid (uint32_t handle);
    BENTLEYDLL_EXPORT static uint32_t     AllocateHandle (void* pointer);
    BENTLEYDLL_EXPORT static void         ReleaseHandle (uint32_t handle);
    BENTLEYDLL_EXPORT static void*        GetPointer (uint32_t handle);
    }; // GlobalHandleContainer

END_BENTLEY_NAMESPACE

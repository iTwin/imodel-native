//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMClassKey.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/**-------------------------------------------------------------------------
    This is the type used to define object IDs, numerical values that
    establish a unique identity tag for a given instance in a given store.
    Each persistent object registered in a store have an object ID.  Each
    persistent object present in physical memory have an object ID if there
    is a saved copy of that instance in a store.  The meaning of the object
    ID is relative to the store whose object is attached to.
*/

typedef uint64_t HPMObjectID;

// This is the object ID to use to establish the absence of an object ID.

#define INVALID_OBJECT_ID ULONG_MAX



END_IMAGEPP_NAMESPACE
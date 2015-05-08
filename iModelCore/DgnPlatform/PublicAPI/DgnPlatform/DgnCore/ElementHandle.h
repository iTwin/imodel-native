/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A list of DgnElementDescr's.
// @bsiclass                                                    Keith.Bentley   02/14
//=======================================================================================
struct DgnElementPtrVec : bvector<DgnElementPtr>
{
const_iterator Find(DgnElementCR val) const
    {
    for (auto it=begin(); it!=end(); ++it)
        {
        if (it->get() == &val)
            return it;
        }
    return end(); // not found
    }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE


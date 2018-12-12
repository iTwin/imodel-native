//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXIndex
//-----------------------------------------------------------------------------
// General class for indexes.
//-----------------------------------------------------------------------------

#pragma once

#include "HIDXIndexable.h"
#include "HIDXSearchCriteria.h"

BEGIN_IMAGEPP_NAMESPACE

typedef enum
    {
    SORTING_NONE,
    SORTING_SIMPLE,
    SORTING_COMPLEX,

    } HIDXSortingRequirement;


template <class T> class DefaultSubIndexType
    {
public:
    bool           SupportsInteractingRetrieval() const
        {
        return false;
        };

    typename HIDXIndexable<T>::List*
    GetInteractingObjects(typename HIDXIndexable<T>::List const& pi_rpObjects) const
        {
        return 0;
        };

    const HFCPtr< HIDXIndexable<T> >
    GetFilledIndexableFor(const HFCPtr< HIDXIndexable<T> >& pi_rpObject) const
        {
        return pi_rpObject;
        };
    };
END_IMAGEPP_NAMESPACE


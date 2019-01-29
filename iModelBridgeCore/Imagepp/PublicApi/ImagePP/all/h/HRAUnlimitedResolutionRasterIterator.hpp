//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAUnlimitedResolutionRasterIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Inline methods for class HRAUnlimitedResolutionRasterIterator
//-----------------------------------------------------------------------------

#include "HRAUnlimitedResolutionRasterIterator.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// Next
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRAUnlimitedResolutionRasterIterator::Next()
    {
    return (m_pRasterIterator->Next());
    }

//-----------------------------------------------------------------------------
// public
// operator()
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRAUnlimitedResolutionRasterIterator::operator()()
    {
    return ((*m_pRasterIterator)());
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
inline void HRAUnlimitedResolutionRasterIterator::Reset()
    {
    m_pRasterIterator->Reset();
    }


END_IMAGEPP_NAMESPACE

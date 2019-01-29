//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPyramidRasterIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------//
// Inline methods for class HRAPyramidRasterIterator
//-----------------------------------------------------------------------------

#include "HRAPyramidRaster.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// Next
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRAPyramidRasterIterator::Next()
    {
    return (m_pRasterIterator->Next());
    }

//-----------------------------------------------------------------------------
// public
// operator()
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRAPyramidRasterIterator::operator()()
    {
    return ((*m_pRasterIterator)());
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
inline void HRAPyramidRasterIterator::Reset()
    {
    m_pRasterIterator->Reset();
    }


END_IMAGEPP_NAMESPACE

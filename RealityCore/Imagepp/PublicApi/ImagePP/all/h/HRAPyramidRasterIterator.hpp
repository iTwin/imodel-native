//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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

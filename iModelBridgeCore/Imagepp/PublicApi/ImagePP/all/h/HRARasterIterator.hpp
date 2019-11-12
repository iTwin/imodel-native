//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRARasterIterator
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns a pointer to the scanned raster.
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRARasterIterator::GetRaster() const
    {
    return m_pRaster;
    }

END_IMAGEPP_NAMESPACE

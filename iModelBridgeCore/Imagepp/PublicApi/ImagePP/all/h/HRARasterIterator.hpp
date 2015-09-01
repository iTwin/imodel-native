//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterIterator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

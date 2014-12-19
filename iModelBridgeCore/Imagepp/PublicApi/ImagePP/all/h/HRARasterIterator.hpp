//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterIterator.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRARasterIterator
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Returns a pointer to the scanned raster.
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRARasterIterator::GetRaster() const
    {
    return m_pRaster;
    }


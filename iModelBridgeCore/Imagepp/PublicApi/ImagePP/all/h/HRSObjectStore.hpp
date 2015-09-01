//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRSObjectStore.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRSObjectStore
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// Returns the PhysicalCoordSys for this store
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HRSObjectStore::GetPhysicalCoordSys() const
    {
    return m_pPhysicalCoordSys;
    }

//-----------------------------------------------------------------------------
// Public
// Returns the LogicalCoordSys for this store
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HRSObjectStore::GetLogicalCoordSys() const
    {
    return m_pLogicalCoordSys;
    }

//-----------------------------------------------------------------------------
// Public
// Indicates if a 1bit background was obtained when the last raster was pulled
//-----------------------------------------------------------------------------
inline bool HRSObjectStore::Has1BitBackgroundColor() const
    {
    return (m_Has1BitBackgroundColor);
    }


//-----------------------------------------------------------------------------
// Public
// Returns the 1 bit background color
//-----------------------------------------------------------------------------
inline uint32_t HRSObjectStore::Get1BitBackgroundColor() const
    {
    return (m_BackgroundColor1Bit);
    }



//-----------------------------------------------------------------------------
// Public
// This method return true if the object store can be loaded as a resizable
// raster. This method must be call before the Load() operation
//-----------------------------------------------------------------------------
inline bool HRSObjectStore::CanBeResizable() const
    {
    return m_pPageDescriptor->IsResizable();
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HRSObjectStore::SetResizableRasterSize(uint64_t pi_RasterWidth,
                                                    uint64_t pi_RasterHeight)
    {
    if (!m_Loaded && CanBeResizable() &&
        (pi_RasterWidth > m_pResDescriptor->GetWidth() || pi_RasterHeight > m_pResDescriptor->GetHeight()))
        {
        m_Resizable = true;
        m_RasterWidth = pi_RasterWidth;
        m_RasterHeight = pi_RasterHeight;
        return true;
        }
    else
        return false;
    }
END_IMAGEPP_NAMESPACE

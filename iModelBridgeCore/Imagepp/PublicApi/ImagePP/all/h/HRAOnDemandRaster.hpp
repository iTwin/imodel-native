//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAOnDemandRaster.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRAOnDemandRaster
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetExtent
//-----------------------------------------------------------------------------
inline HGF2DExtent HRAOnDemandRaster::GetExtent() const
    {
    // Return extent constructed from our shape
    return GetEffectiveShape()->GetExtent();
    }


//-----------------------------------------------------------------------------
// public
// GetEffectiveShape - Return the intersection between the physical and logical
//                     shape.
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape> HRAOnDemandRaster::GetEffectiveShape () const
    {
    HPRECONDITION(m_pEffectiveShape != 0);
    return m_pEffectiveShape;
    }

//-----------------------------------------------------------------------------
// public
// GetRepresentativePSS - Return the representative PSS.
//-----------------------------------------------------------------------------
inline WString HRAOnDemandRaster::GetRepresentativePSS() const
    {
    return m_RepresentativePSS;
    }

//-----------------------------------------------------------------------------
// public
// HasLookAhead - Return true if the raster is supporting the lookahead.
//-----------------------------------------------------------------------------
inline bool HRAOnDemandRaster::HasLookAhead() const
    {
    return m_hasLookAhead;
    }

//-----------------------------------------------------------------------------
// public
// IsDataChangingWithResolution - Return true if the raster data are changing 
//                                depending on the resolution requested.
//-----------------------------------------------------------------------------
inline bool HRAOnDemandRaster::IsDataChangingWithResolution() const
    {
    return m_isDataChangingWithResolution;
    }

//-----------------------------------------------------------------------------
// public
// HasUnlimitedRasterSource - Return true if the raster has some unlimited 
//                            raster source. 
//-----------------------------------------------------------------------------
inline bool HRAOnDemandRaster::HasUnlimitedRasterSource() const
    {
    return m_hasUnlimitedRasterSource;
    }

//-----------------------------------------------------------------------------
// public
// HasLastLoadFailed - Return true if the last loading of the raster failed.
//-----------------------------------------------------------------------------
inline bool HRAOnDemandRaster::HasLastLoadFailed() const
    {
    return m_hasLastLoadFailed;
    }
END_IMAGEPP_NAMESPACE

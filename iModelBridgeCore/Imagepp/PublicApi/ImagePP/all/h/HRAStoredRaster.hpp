//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAStoredRaster.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRAStoredRaster
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// HasSinglePixelType - Raster has a single pixel type.
//-----------------------------------------------------------------------------
inline bool HRAStoredRaster::HasSinglePixelType() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// public
// IsStoredRaster
//-----------------------------------------------------------------------------
inline bool HRAStoredRaster::IsStoredRaster () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// public
// GetPhysicalCoordSys -
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DCoordSys>& HRAStoredRaster::GetPhysicalCoordSys () const
    {
    return (m_pPhysicalCoordSys);
    }

//-----------------------------------------------------------------------------
// public
// GetTransFoModel -  Get the model between the Physical and Logical CoordSys
//      Use the ConvertDirect to transform Physical to Logical
//      Use the ConvertInverse to transform Logical to Physical
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DTransfoModel>& HRAStoredRaster::GetTransfoModel () const
    {
    return (m_pTransfoModel);
    }

//-----------------------------------------------------------------------------
// public
// GetSize -  Return the current physical shape.
//-----------------------------------------------------------------------------
inline HGF2DExtent HRAStoredRaster::GetPhysicalExtent() const
    {
    HPRECONDITION(m_pPhysicalRect->IsRectangle());

    return m_pPhysicalRect->GetExtent();
    }

//-----------------------------------------------------------------------------
// public
// GetEffectiveShape - Return the intersection between the physical and logical
//                     shape.
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape> HRAStoredRaster::GetEffectiveShape () const
    {
    return m_pEffectiveShape;
    }

//-----------------------------------------------------------------------------
// public
// IsResizable
//-----------------------------------------------------------------------------
inline bool HRAStoredRaster::IsResizable() const
    {
    return m_Resizable;
    }

//-----------------------------------------------------------------------------
// public
// GetRasterExtent
//-----------------------------------------------------------------------------
inline HGF2DExtent HRAStoredRaster::GetRasterExtent() const
    {
    return m_pRasterPhysicalRect->GetExtent();
    }

//-----------------------------------------------------------------------------
// public
// InvalidateRaster : Invalidate the memory data related to this raster
//-----------------------------------------------------------------------------
inline void HRAStoredRaster::InvalidateRaster()
    {
    //Classes derived from HRAStoredRaster should defined this method if
    //there is chance that it ever get called on these derived classes.
    HASSERT(0);
    }

END_IMAGEPP_NAMESPACE

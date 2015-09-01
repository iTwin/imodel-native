//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapBase.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//
// Class: HRABitmapBase
// ----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

// ----------------------------------------------------------------------------
// public
// SetPosInRaster
// ----------------------------------------------------------------------------
inline void HRABitmapBase::SetPosInRaster(HUINTX     pi_PosX,
                                          HUINTX     pi_PosY)
    {
    m_XPosInRaster = pi_PosX;
    m_YPosInRaster = pi_PosY;
    }

// ----------------------------------------------------------------------------
// public
// GetPosInRaster
// ----------------------------------------------------------------------------
inline void HRABitmapBase::GetPosInRaster(HUINTX*    po_pPosX,
                                          HUINTX*    po_pPosY) const
    {
    *po_pPosX = m_XPosInRaster;
    *po_pPosY = m_YPosInRaster;
    }


END_IMAGEPP_NAMESPACE
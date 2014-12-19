//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPreDrawOptions.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HRAPreDrawOptions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// GetDrawSurfaceMaxDimension
//-----------------------------------------------------------------------------
inline void HRAPreDrawOptions::GetDrawSurfaceMaxDimension(uint32_t& po_rDrawSurfaceMaxWidth,
                                                          uint32_t& po_rDrawSurfaceMaxHeight)
    {
    po_rDrawSurfaceMaxWidth = m_DrawSurfaceMaxWidth;
    po_rDrawSurfaceMaxHeight = m_DrawSurfaceMaxHeight;
    }


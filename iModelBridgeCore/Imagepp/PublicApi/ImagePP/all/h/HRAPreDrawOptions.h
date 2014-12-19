//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPreDrawOptions.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRADrawOptions
//-----------------------------------------------------------------------------

#pragma once


class HRAPreDrawOptions : public HFCShareableObject<HRAPreDrawOptions>
    {
public:

    // Primary methods
    HRAPreDrawOptions(uint32_t pi_DrawSurfaceMaxWidth,
                      uint32_t pi_DrawSurfaceMaxHeight);

    HRAPreDrawOptions(const HRAPreDrawOptions& pi_rOptions);

    virtual                 ~HRAPreDrawOptions();

    // Operators

    HRAPreDrawOptions&        operator=(const HRAPreDrawOptions& pi_rObj);

    void                    GetDrawSurfaceMaxDimension(uint32_t& po_rDrawSurfaceMaxWidth,
                                                       uint32_t& po_rDrawSurfaceMaxHeight);

private:

    uint32_t                      m_DrawSurfaceMaxWidth;
    uint32_t                      m_DrawSurfaceMaxHeight;
    };

#include "HRAPreDrawOptions.hpp"


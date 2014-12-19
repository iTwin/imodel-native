//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceDescriptor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSSurfaceDescriptor
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HGSSurfaceCapabilities.h"

class HGSSurfaceDescriptor : public HFCShareableObject<HGSSurfaceDescriptor>
    {
    // HChk HL Change the ID
    HDECLARE_BASECLASS_ID(1721)

public:

    virtual         ~HGSSurfaceDescriptor();

    virtual HGSSurfaceCapabilities*
    GetRequiredSurfaceCapabilities() const;

    uint32_t        GetWidth() const;
    uint32_t        GetHeight() const;
    uint32_t        GetDataWidth() const;
    uint32_t        GetDataHeight() const;


    // To be used only internally.
    void            SetDimensions(uint32_t pi_Width, uint32_t pi_Height);
    void            SetDataDimensions(uint32_t pi_Width, uint32_t pi_Height);

    virtual double GetWidthToHeightUnitRatio() const;

    void            SetOffsets  (HUINTX     pi_OffsetX,
                                 HUINTX     pi_OffsetY);
    void            GetOffsets  (HUINTX*    po_pOffsetX,
                                 HUINTX*    po_pOffsetY) const;

protected:

    HGSSurfaceDescriptor(uint32_t                        pi_Width,
                         uint32_t                        pi_Height);


private:

    // private members
    HUINTX          m_OffsetX;
    HUINTX          m_OffsetY;
    uint32_t        m_Width;
    uint32_t        m_Height;
    uint32_t        m_DataWidth;
    uint32_t        m_DataHeight;

    // disabled methods
    HGSSurfaceDescriptor();
    HGSSurfaceDescriptor(const HGSSurfaceDescriptor& pi_rObj);
    HGSSurfaceDescriptor&
    operator=(const HGSSurfaceDescriptor& pi_rObj);
    };

#include "HGSSurfaceDescriptor.hpp"


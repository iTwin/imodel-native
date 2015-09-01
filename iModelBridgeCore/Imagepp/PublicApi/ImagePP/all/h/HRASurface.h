//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASurface.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRASurface
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HGSSurfaceDescriptor;
class HGSRegion;

class HRASurface
    {
    HDECLARE_BASECLASS_ID(HRASurfaceId)

public:
    // Primary methods
    HRASurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor);

    virtual ~HRASurface();

    const HFCPtr<HGSRegion>& GetRegion() const;

    void SetRegion(const HFCPtr<HGSRegion>& pRegion);

    // description of the surface
    const HFCPtr<HGSSurfaceDescriptor>& GetSurfaceDescriptor() const;

protected:

private:
    
    // private members
    HFCPtr<HGSRegion>               m_pRegion;
    HFCPtr<HGSSurfaceDescriptor>    m_pSurfaceDescriptor;

    // disabled methods
    HRASurface(const HRASurface& pi_rObj);
    HRASurface&
    operator=(const HRASurface& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

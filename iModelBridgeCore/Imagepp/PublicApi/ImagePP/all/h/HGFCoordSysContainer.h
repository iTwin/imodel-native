//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFCoordSysContainer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFCoordSysContainer
//-----------------------------------------------------------------------------
// Description of 2D coordinate system. This class implements the context in
// which graphical and positional object are interpreted.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DCoordSys;

class HGFCoordSysContainer : public HFCShareableObject<HGFCoordSysContainer>
    {
    HDECLARE_BASECLASS_ID(HGFCoordSysContainerId_Base)

public:

    // Primary methods
    HGFCoordSysContainer();
    HGFCoordSysContainer(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HGFCoordSysContainer(const HGFCoordSysContainer&      pi_rObj);
    virtual         ~HGFCoordSysContainer();

    HGFCoordSysContainer&  operator=(const HGFCoordSysContainer& pi_rObj);

    // get and set methos
    HFCPtr<HGF2DCoordSys>
    GetCoordSys() const;
    void            SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

protected:

private:

    // referenced coordsys
    HFCPtr<HGF2DCoordSys>
    m_pCoordSys;
    };


END_IMAGEPP_NAMESPACE
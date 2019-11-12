//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAReferenceToRaster
//-----------------------------------------------------------------------------
// This class describes a reference to another raster object.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAReferenceToRaster.h"
#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
class HRAStoredRaster;
class HMGMessage;

class HRAReferenceToStoredRaster : public HRAReferenceToRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAReferenceToStoredRasterId)

public:
    DEFINE_T_SUPER(HRAReferenceToRaster)

    // Primary methods

    IMAGEPP_EXPORT HRAReferenceToStoredRaster();     // Do not call directly

    IMAGEPP_EXPORT HRAReferenceToStoredRaster(const HFCPtr<HRAStoredRaster>& pi_pSource);

    IMAGEPP_EXPORT virtual ~HRAReferenceToStoredRaster();


    // From HRAReferenceToRaster
    void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys) override;

    // Message handlers
    bool NotifyGeometryChanged       (const HMGMessage& pi_rMessage) override;


protected:
    HFCPtr<HGF2DTransfoModel> m_pPhysicalToLogical;
    };
END_IMAGEPP_NAMESPACE

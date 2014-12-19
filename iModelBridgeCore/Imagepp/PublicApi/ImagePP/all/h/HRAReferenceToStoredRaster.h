//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAReferenceToStoredRaster.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAReferenceToRaster
//-----------------------------------------------------------------------------
// This class describes a reference to another raster object.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAReferenceToRaster.h"
#include "HFCPtr.h"

class HRAStoredRaster;
class HMGMessage;

class HRAReferenceToStoredRaster : public HRAReferenceToRaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1374)

public:
    DEFINE_T_SUPER(HRAReferenceToRaster)

    // Primary methods

    _HDLLg HRAReferenceToStoredRaster();     // Do not call directly

    _HDLLg HRAReferenceToStoredRaster(const HFCPtr<HRAStoredRaster>& pi_pSource);

    _HDLLg virtual ~HRAReferenceToStoredRaster();


    // From HRAReferenceToRaster
    virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys);

    // Message handlers
    virtual bool NotifyGeometryChanged       (const HMGMessage& pi_rMessage);


protected:
    HFCPtr<HGF2DTransfoModel> m_pPhysicalToLogical;
    };
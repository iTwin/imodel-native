//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DCameraTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DCameraTransfoModel
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGFCameraData.h"
#include "HGF3DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
// HPM_DECLARE_HEADER(HGF3DCameraTransfoModel)

class HGF3DCameraTransfoModel: public HGF3DTransfoModel
    {


public:

//      HPM_DECLARE_CLASS(98773)

    // Primary methods
    HGF3DCameraTransfoModel(const HGFCameraData& pi_rCameraData);
    HGF3DCameraTransfoModel(const HGF3DCameraTransfoModel& pi_rObj);
    HGF3DCameraTransfoModel&
    operator=(const HGF3DCameraTransfoModel& pi_rObj);



    virtual           ~HGF3DCameraTransfoModel();

    // Conversion interface
    virtual void      ConvertDirect(double*   pio_pXInOut,
                                    double*   pio_pYInOut,
                                    double*   pio_pZInOut) const;

    virtual void      ConvertInverse(double*   pio_pXInOut,
                                     double*   pio_pYInOut,
                                     double*   pio_pZInOut) const;

    virtual void      ConvertDirect(double    pi_XIn,
                                    double    pi_YIn,
                                    double    pi_ZIn,
                                    double*   po_pXOut,
                                    double*   po_pYOut,
                                    double*   po_pZOut) const;

    virtual void      ConvertInverse(double    pi_XIn,
                                     double    pi_YIn,
                                     double    pi_ZIn,
                                     double*   po_pXOut,
                                     double*   po_pYOut,
                                     double*   po_pZOut) const;


protected:

private:
    HGFCameraData             m_CameraData;

    HFCMatrix<4, 4, double>     m_RotationMatrix;
    HFCMatrix<4, 4, double>     m_InvRotationMatrix;



    };

END_IMAGEPP_NAMESPACE
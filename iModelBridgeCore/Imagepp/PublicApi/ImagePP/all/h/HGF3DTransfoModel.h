//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DTransfoModel
//-----------------------------------------------------------------------------
// Description of 3D transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF3DTransfoModel
    {


public:


    virtual           ~HGF3DTransfoModel();

    // Conversion interface
    virtual void      ConvertDirect(double*   pio_pXInOut,
                                    double*   pio_pYInOut,
                                    double*   pio_pZInOut) const = 0;

    virtual void      ConvertDirect(double    pi_XIn,
                                    double    pi_YIn,
                                    double    pi_ZIn,
                                    double*   po_pXOut,
                                    double*   po_pYOut,
                                    double*   po_pZOut) const;


    virtual void      ConvertInverse(double*   pio_pXInOut,
                                     double*   pio_pYInOut,
                                     double*   pio_pZInOut) const = 0;

    virtual void      ConvertInverse(double    pi_XIn,
                                     double    pi_YIn,
                                     double    pi_ZIn,
                                     double*   po_pXOut,
                                     double*   po_pYOut,
                                     double*   po_pZOut) const;



    void StudyReversibilityPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                         double                pi_ZMin,
                                         double                pi_ZMax,
                                         double                pi_Step,
                                         double*               po_pMeanError,
                                         double*               po_pMaxError) const;

protected:
    // Primary methods
    HGF3DTransfoModel();
    HGF3DTransfoModel(const HGF3DTransfoModel& pi_rObj);
    HGF3DTransfoModel&
    operator=(const HGF3DTransfoModel& pi_rObj);


private:


    };

END_IMAGEPP_NAMESPACE
#include "HGF3DTransfoModel.hpp"


//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLiteTolerance.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFLiteTolerance
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DLiteQuadrilateral.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DTransfoModel;


#define DEFAULT_PIXEL_TOLERANCE (0.125)

class HGFLiteTolerance : public HFCShareableObject<HGFLiteTolerance>
    {
public:

    // Primary methods
    IMAGEPP_EXPORT                   HGFLiteTolerance();
    IMAGEPP_EXPORT                   HGFLiteTolerance(double pi_x0, double pi_y0,
                                                     double pi_x1, double pi_y1,
                                                     double pi_x2, double pi_y2,
                                                     double pi_x3, double pi_y3);

    IMAGEPP_EXPORT                   HGFLiteTolerance(double pi_XMin, double pi_YMin,
                                                     double pi_xMax, double pi_yMax);

    IMAGEPP_EXPORT                   HGFLiteTolerance(const HGFLiteTolerance& pi_rObj);
    IMAGEPP_EXPORT virtual           ~HGFLiteTolerance();

    HGFLiteTolerance&               operator=(const HGFLiteTolerance& pi_rObj);

    double                          GetLinearTolerance () const;

    bool                            TransformDirect(const HGF2DTransfoModel& pi_rModel);
    bool                            TransformInverse(const HGF2DTransfoModel& pi_rModel);

private:
    double                m_XMin;
    double                m_YMin;

    // Coordinate of the tolerance polygon
    HGF2DLiteQuadrilateral m_Quadrilateral;

    mutable double        m_Tolerance;
    };


END_IMAGEPP_NAMESPACE
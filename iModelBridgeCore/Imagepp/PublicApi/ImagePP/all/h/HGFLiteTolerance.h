//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLiteTolerance.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFLiteTolerance
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/h/HmrTypes.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DLiteQuadrilateral.h>


#define DEFAULT_PIXEL_TOLERANCE (0.125)

class HGFLiteTolerance : public HFCShareableObject<HGFLiteTolerance>
    {
public:

    // Primary methods
    _HDLLg                     HGFLiteTolerance();
    _HDLLg                     HGFLiteTolerance(double pi_x0, double pi_y0,
                                            double pi_x1, double pi_y1,
                                            double pi_x2, double pi_y2,
                                            double pi_x3, double pi_y3);

    _HDLLg                    HGFLiteTolerance(double pi_XMin, double pi_YMin,
                                               double pi_xMax, double pi_yMax);

    _HDLLg                     HGFLiteTolerance(const HGFLiteTolerance& pi_rObj);
    _HDLLg virtual            ~HGFLiteTolerance();

    HGFLiteTolerance&       operator=(const HGFLiteTolerance& pi_rObj);

    double         GetLinearTolerance () const;

private:
    double                m_XMin;
    double                m_YMin;

    // Coordinate of the tolerance polygon
    HGF2DLiteQuadrilateral m_Quadrilateral;

    mutable double        m_Tolerance;
    };



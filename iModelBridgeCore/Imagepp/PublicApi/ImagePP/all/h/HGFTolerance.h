//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFTolerance.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFTolerance
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/h/HmrTypes.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DLocation.h>
#include <Imagepp/all/h/HGF2DLiteQuadrilateral.h>


#define DEFAULT_PIXEL_TOLERANCE (0.125)

class HGFTolerance : public HFCShareableObject<HGFTolerance>
    {
public:

    // Primary methods
    _HDLLg                     HGFTolerance();
    _HDLLg                     HGFTolerance(double pi_x0, double pi_y0,
                                            double pi_x1, double pi_y1,
                                            double pi_x2, double pi_y2,
                                            double pi_x3, double pi_y3,
                                            HFCPtr<HGF2DCoordSys>  pi_pCoordSys);

    _HDLLg                    HGFTolerance(double pi_XMin, double pi_YMin,
                                           double pi_xMax, double pi_yMax,
                                           HFCPtr<HGF2DCoordSys>  pi_pCoordSys);

    _HDLLg                     HGFTolerance(const HGFTolerance& pi_rObj);
    _HDLLg virtual            ~HGFTolerance();

    HGFTolerance&       operator=(const HGFTolerance& pi_rObj);

    HGFTolerance&       ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HGFTolerance&       SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    double              GetLinearTolerance () const;

private:
    double                m_XMin;
    double                m_YMin;

    // Coordinate of the tolerance polygon
    HGF2DLiteQuadrilateral m_Quadrilateral;

    mutable double        m_Tolerance;

    // Coordinate system
    HFCPtr<HGF2DCoordSys>  m_pCoordSys;
    };



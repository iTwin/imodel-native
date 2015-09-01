//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteQuadrilateral.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DPosition.h>

BEGIN_IMAGEPP_NAMESPACE
class HGF2DLiteQuadrilateral
    {
public:
    HGF2DLiteQuadrilateral();

    HGF2DLiteQuadrilateral(double pi_x0, double pi_y0, double pi_x1, double pi_y1, double pi_x2, double pi_y2, double pi_x3, double pi_y3) ;

    HGF2DLiteQuadrilateral& operator= (HGF2DLiteQuadrilateral const& pi_rObj);


    void                GetCorners(double* po_x0, double* po_y0, double* po_x1, double* po_y1,
                                   double* po_x2, double* po_y2, double* po_x3, double* po_y3) const;

    bool                IsPointIn      (double x, double y) const;
    bool                IsPointOuterIn (double x, double y) const;

    HGF2DLiteExtent const&
    GetExtent() const;

private:
    double             CalcAngle  (double x0, double y0, double x1, double y1,
                                    double x2, double y2, double x3, double y3) const;

    HGF2DPosition       CalcCentroid(double x0, double y0,
                                     double x1, double y1,
                                     double x2, double y2) const;

    double             CalcDeterminant (double x0, double y0, double x1, double y1, double x2, double y2) const;

    double             CalcEpsilon() const;

    double             CalcLength (double x0, double y0, double x1, double y1) const;

    HGF2DPosition       CalcOuterPoint (double x0, double y0,
                                        double x1, double y1,
                                        double x2, double y2,
                                        double pi_Epsilon ) const;

    void                ComputeOuterQuadrilatere() const;

    double             DotProduct (double x0, double y0, double x1, double y1) const;

    double             Square (double x) const;

    // Members
    mutable double m_OuterX0;
    mutable double m_OuterY0;
    mutable double m_OuterX1;
    mutable double m_OuterY1;
    mutable double m_OuterX2;
    mutable double m_OuterY2;
    mutable double m_OuterX3;
    mutable double m_OuterY3;

    double         m_x0;
    double         m_y0;
    double         m_x1;
    double         m_y1;
    double         m_x2;
    double         m_y2;
    double         m_x3;
    double         m_y3;

    double         m_CentroidX;
    double         m_CentroidY;

    double         m_TwiceArea;

    HGF2DLiteExtent m_Extent;

    mutable bool m_OuterQuadrilateralDefined;
    };

END_IMAGEPP_NAMESPACE

#include <Imagepp/all/h/HGF2DLiteQuadrilateral.hpp>


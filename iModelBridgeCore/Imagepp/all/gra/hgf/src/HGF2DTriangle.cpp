//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DTriangle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGF2DTriangle.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGF2DTriangle::HGF2DTriangle
(
    double pi_x0,
    double pi_y0,
    double pi_x1,
    double pi_y1,
    double pi_x2,
    double pi_y2
)
    {
    // Compute extent before centering the triangle
    HGF2DPosition Origin(MIN(MIN(pi_x0,pi_x1), pi_x2), MIN(MIN(pi_y0,pi_y1), pi_y2));
    HGF2DPosition Corner(MAX(MAX(pi_x0,pi_x1), pi_x2), MAX(MAX(pi_y0,pi_y1), pi_y2));
    m_Extent = HGF2DLiteExtent(Origin, Corner);

    // Compute the original triangle centroid
    m_CentroidX = (pi_x0 + pi_x1 + pi_x2) / 3.0;
    m_CentroidY = (pi_y0 + pi_y1 + pi_y2) / 3.0;

    // Compute a triangle centered at (0,0)
    m_x0 = pi_x0 - m_CentroidX;
    m_y0 = pi_y0 - m_CentroidY;
    m_x1 = pi_x1 - m_CentroidX;
    m_y1 = pi_y1 - m_CentroidY;
    m_x2 = pi_x2 - m_CentroidX;
    m_y2 = pi_y2 - m_CentroidY;

    // Calc Area (The offset doesn't change the area but make it more acurate)
    m_TwiceArea = CalcDeterminant(m_x0, m_y0, m_x1, m_y1, m_x2, m_y2);

    if (m_TwiceArea < 0.0)
        {
        // Reverse points
        m_x2 = pi_x0 - m_CentroidX;
        m_y2 = pi_y0 - m_CentroidY;
        m_x1 = pi_x1 - m_CentroidX;
        m_y1 = pi_y1 - m_CentroidY;
        m_x0 = pi_x2 - m_CentroidX;
        m_y0 = pi_y2 - m_CentroidY;
        m_TwiceArea *= -1.0;
        }

    m_OuterTriangleDefined = false;

    // Check for imprecision
    // Activate this assertion if some imprecision problems seems to occur.
    // HASSERT(HDOUBLE_EQUAL_EPSILON(CalcDeterminant(m_x0, m_y0, m_x1, m_y1, m_x2, m_y2), CalcDeterminant(m_x2, m_y2, m_x0, m_y0, m_x1, m_y1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DTriangle::IsPointIn
(
    double x,
    double y
) const
    {
    // Check extent first
    if (m_Extent.IsPointIn(HGF2DPosition(x, y)))
        {
        x -= m_CentroidX;
        y -= m_CentroidY;

        return m_TwiceArea > 0.0 && CalcDeterminant(x, y, m_x0, m_y0, m_x1, m_y1) >= 0.0 &&
               CalcDeterminant(x, y, m_x1, m_y1, m_x2, m_y2) >= 0.0 &&
               CalcDeterminant(x, y, m_x2, m_y2, m_x0, m_y0) >= 0.0;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DTriangle::IsPointOuterIn
(
    double x,
    double y
) const
    {
    // Check extent first
    if (m_Extent.IsPointOutterIn(HGF2DPosition(x, y)))
        {
        // Compute the outer triangle
        if (!m_OuterTriangleDefined)
            ComputeOuterTriangle();

        x -= m_CentroidX;
        y -= m_CentroidY;

        return m_TwiceArea > 0.0 && CalcDeterminant(x, y, m_OuterX0, m_OuterY0, m_OuterX1, m_OuterY1) >= 0.0 &&
               CalcDeterminant(x, y, m_OuterX1, m_OuterY1, m_OuterX2, m_OuterY2) >= 0.0 &&
               CalcDeterminant(x, y, m_OuterX2, m_OuterY2, m_OuterX0, m_OuterY0) >= 0.0;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
double HGF2DTriangle::CalcAngle (double x0, double y0, double x1, double y1,
                                  double x2, double y2, double x3, double y3) const
    {
    double V0x = x1-x0;
    double V0y = y1-y0;
    double V1x = x3-x2;
    double V1y = y3-y2;

    // Calculate unit vector
    double Length0 (CalcLength(x0,y0,x1,y1));
    double Length1 (CalcLength(x2,y2,x3,y3));

    // Normalize
    V0x /= Length0;
    V0y /= Length0;
    V1x /= Length1;
    V1y /= Length1;

    return acos(DotProduct(V0x, V0y, V1x, V1y));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGF2DPosition HGF2DTriangle::CalcOuterPoint (double x0, double y0,
                                             double x1, double y1,
                                             double x2, double y2,
                                             double pi_Epsilon) const
    {
    double const CentroidX(0.0);
    double const CentroidY(0.0);

    double Angle = CalcAngle(x0, y0, x1, y1, x0, y0, x2, y2) / 2.0;
    double Tol(pi_Epsilon / sin(Angle));
    double Length (CalcLength(CentroidX, CentroidY, x0, y0));
    double Dx (x0 / Length);
    double Dy (y0 / Length);
    Dx *= Tol;
    Dy *= Tol;

    return HGF2DPosition(x0 + Dx, y0 + Dy);
    }

/*---------------------------------------------------------------------------------**//**
* The computation of the outer triangle assumes that the inner triangle is centered at (0,0)
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
void HGF2DTriangle::ComputeOuterTriangle() const
    {
    double Epsilon(CalcEpsilon());

    HGF2DPosition p0 (CalcOuterPoint(m_x0, m_y0, m_x1, m_y1, m_x2, m_y2, Epsilon));
    HGF2DPosition p1 (CalcOuterPoint(m_x1, m_y1, m_x2, m_y2, m_x0, m_y0, Epsilon));
    HGF2DPosition p2 (CalcOuterPoint(m_x2, m_y2, m_x0, m_y0, m_x1, m_y1, Epsilon));

    m_OuterX0 = p0.GetX();
    m_OuterY0 = p0.GetY();

    m_OuterX1 = p1.GetX();
    m_OuterY1 = p1.GetY();

    m_OuterX2 = p2.GetX();
    m_OuterY2 = p2.GetY();

    m_OuterTriangleDefined = true;

    // The outer triangle must me greater than the original
    HASSERT(CalcDeterminant(m_OuterX0, m_OuterY0, m_OuterX1, m_OuterY1, m_OuterX2, m_OuterY2) > m_TwiceArea);
    }

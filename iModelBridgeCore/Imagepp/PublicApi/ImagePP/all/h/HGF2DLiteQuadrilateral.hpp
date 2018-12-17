//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteQuadrilateral.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline HGF2DLiteQuadrilateral::HGF2DLiteQuadrilateral
(
)
    {
    m_CentroidX = 0.0;
    m_CentroidY = 0.0;

    m_x0 = m_y0 = 0.0;
    m_x1 = m_y1 = 0.0;
    m_x2 = m_y2 = 0.0;
    m_x3 = m_y3 = 0.0;

    m_TwiceArea = 0.0;

    m_OuterQuadrilateralDefined = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline HGF2DLiteQuadrilateral& HGF2DLiteQuadrilateral::operator=
(
    HGF2DLiteQuadrilateral const& pi_rObj
)
    {
    if (this != &pi_rObj)
        {
        m_x0 = pi_rObj.m_x0, m_y0 = pi_rObj.m_y0;
        m_x1 = pi_rObj.m_x1, m_y1 = pi_rObj.m_y1;
        m_x2 = pi_rObj.m_x2, m_y2 = pi_rObj.m_y2;
        m_x3 = pi_rObj.m_x3, m_y3 = pi_rObj.m_y3;

        m_TwiceArea = pi_rObj.m_TwiceArea;

        m_CentroidX = pi_rObj.m_CentroidX;
        m_CentroidY = pi_rObj.m_CentroidY;

        if (pi_rObj.m_OuterQuadrilateralDefined)
            {
            m_OuterX0 = pi_rObj.m_OuterX0;
            m_OuterY0 = pi_rObj.m_OuterY0;
            m_OuterX1 = pi_rObj.m_OuterX1;
            m_OuterY1 = pi_rObj.m_OuterY1;
            m_OuterX2 = pi_rObj.m_OuterX2;
            m_OuterY2 = pi_rObj.m_OuterY2;
            m_OuterX3 = pi_rObj.m_OuterX3;
            m_OuterY3 = pi_rObj.m_OuterY3;
            }

        m_Extent = pi_rObj.m_Extent;

        m_OuterQuadrilateralDefined = pi_rObj.m_OuterQuadrilateralDefined;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline HGF2DPosition HGF2DLiteQuadrilateral::CalcCentroid(double x0, double y0,
                                                          double x1, double y1,
                                                          double x2, double y2) const
    {
    return HGF2DPosition((x0 + x1 + x2) / 3.0, (y0 + y1 + y2) / 3.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline double HGF2DLiteQuadrilateral::CalcDeterminant
(
    double x0,
    double y0,
    double x1,
    double y1,
    double x2,
    double y2
) const
    {
    return (x0*(y1-y2) - y0*(x1-x2) + ((x1*y2) - (x2*y1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline double HGF2DLiteQuadrilateral::CalcLength (double x0, double y0, double x1, double y1) const
    {
    return sqrt(Square(x1-x0)+(Square(y1-y0)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline double HGF2DLiteQuadrilateral::DotProduct(double x0, double y0, double x1, double y1) const
    {
    return x0*x1 + y0*y1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline double HGF2DLiteQuadrilateral::Square (double x) const
    {
    return x * x;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline void HGF2DLiteQuadrilateral::GetCorners
(
    double* po_x0,
    double* po_y0,
    double* po_x1,
    double* po_y1,
    double* po_x2,
    double* po_y2,
    double* po_x3,
    double* po_y3
) const
    {
    *po_x0 = m_x0 + m_CentroidX;
    *po_y0 = m_y0 + m_CentroidY;
    *po_x1 = m_x1 + m_CentroidX;
    *po_y1 = m_y1 + m_CentroidY;
    *po_x2 = m_x2 + m_CentroidX;
    *po_y2 = m_y2 + m_CentroidY;
    *po_x3 = m_x3 + m_CentroidX;
    *po_y3 = m_y3 + m_CentroidY;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline HGF2DLiteExtent const& HGF2DLiteQuadrilateral::GetExtent
(
) const
    {
    return m_Extent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
inline double HGF2DLiteQuadrilateral::CalcEpsilon() const
    {
    double xMax(MAX(MAX(fabs(m_x0),fabs(m_x1)), MAX(fabs(m_x2),fabs(m_x3))));
    double yMax(MAX(MAX(fabs(m_y0),fabs(m_y1)), MAX(fabs(m_y2),fabs(m_y3))));

    return MAX(xMax, yMax) * HEPSILON_MULTIPLICATOR;
    }
END_IMAGEPP_NAMESPACE
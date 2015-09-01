//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFTolerance.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFTolerance
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFTolerance.h>


#define SQUARE(a) ((a)*(a))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance::HGFTolerance ()
    : m_XMin(0.0), m_YMin(0.0), m_Quadrilateral(), m_Tolerance(-1.0),
      m_pCoordSys(new HGF2DCoordSys())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance::HGFTolerance
(
    double pi_x0,
    double pi_y0,
    double pi_x1,
    double pi_y1,
    double pi_x2,
    double pi_y2,
    double pi_x3,
    double pi_y3,
    HFCPtr<HGF2DCoordSys>  pi_pCoordSys
)
    : m_XMin(MIN(MIN(pi_x0, pi_x1), MIN(pi_x2, pi_x3))),
      m_YMin(MIN(MIN(pi_y0, pi_y1), MIN(pi_y2, pi_y3))),
      m_Quadrilateral(pi_x0 - m_XMin, pi_y0 - m_YMin,
                      pi_x1 - m_XMin, pi_y1 - m_YMin,
                      pi_x2 - m_XMin, pi_y2 - m_YMin,
                      pi_x3 - m_XMin, pi_y3 - m_YMin),
      m_Tolerance (-1.0),
      m_pCoordSys(pi_pCoordSys)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance::HGFTolerance
(
    double pi_XMin,
    double pi_YMin,
    double pi_XMax,
    double pi_YMax,
    HFCPtr<HGF2DCoordSys>  pi_pCoordSys
)
    : m_XMin(pi_XMin),
      m_YMin(pi_YMin),
      m_Quadrilateral(pi_XMin - m_XMin, pi_YMin - m_YMin,
                      pi_XMin - m_XMin, pi_YMax - m_YMin,
                      pi_XMax - m_XMin, pi_YMax - m_YMin,
                      pi_XMax - m_XMin, pi_YMin - m_YMin),
      m_Tolerance (-1.0),
      m_pCoordSys(pi_pCoordSys)
    {
    HASSERT(pi_XMin < pi_XMax);
    HASSERT(pi_YMin < pi_YMax);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance::HGFTolerance
(
    const HGFTolerance& pi_rObj
)
    {
    m_Quadrilateral = pi_rObj.m_Quadrilateral;
    m_XMin = pi_rObj.m_XMin;
    m_YMin = pi_rObj.m_YMin;
    m_Tolerance = pi_rObj.m_Tolerance;
    m_pCoordSys = pi_rObj.m_pCoordSys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance::~HGFTolerance
(
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance& HGFTolerance::operator=
(
    const HGFTolerance& pi_rObj
)
    {
    if (this != &pi_rObj)
        {
        m_Quadrilateral = pi_rObj.m_Quadrilateral;
        m_XMin = pi_rObj.m_XMin;
        m_YMin = pi_rObj.m_YMin;
        m_Tolerance = pi_rObj.m_Tolerance;
        m_pCoordSys = pi_rObj.m_pCoordSys;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance& HGFTolerance::ChangeCoordSys
(
    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys
)
    {
    if (pi_rpCoordSys != m_pCoordSys)
        {
        double x0, y0, x1, y1, x2, y2, x3, y3;

        m_Quadrilateral.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

        HGF2DLocation Point0(x0 + m_XMin, y0 + m_YMin, m_pCoordSys);
        HGF2DLocation Point1(x1 + m_XMin, y1 + m_YMin, m_pCoordSys);
        HGF2DLocation Point2(x2 + m_XMin, y2 + m_YMin, m_pCoordSys);
        HGF2DLocation Point3(x3 + m_XMin, y3 + m_YMin, m_pCoordSys);

        Point0.ChangeCoordSys(pi_rpCoordSys);
        Point1.ChangeCoordSys(pi_rpCoordSys);
        Point2.ChangeCoordSys(pi_rpCoordSys);
        Point3.ChangeCoordSys(pi_rpCoordSys);

        x0 = Point0.GetX();
        y0 = Point0.GetY();
        x1 = Point1.GetX();
        y1 = Point1.GetY();
        x2 = Point2.GetX();
        y2 = Point2.GetY();
        x3 = Point3.GetX();
        y3 = Point3.GetY();

        m_XMin = MIN(MIN(x0, x1), MIN(x2, x3));
        m_YMin = MIN(MIN(y0, y1), MIN(y2, y3));

        m_Quadrilateral = HGF2DLiteQuadrilateral(x0 - m_XMin, y0 - m_YMin,
                                                 x1 - m_XMin, y1 - m_YMin,
                                                 x2 - m_XMin, y2 - m_YMin,
                                                 x3 - m_XMin, y3 - m_YMin);
        m_pCoordSys = pi_rpCoordSys;

        m_Tolerance = -1.0;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFTolerance& HGFTolerance::SetCoordSys
(
    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys
)
    {
    if (pi_rpCoordSys != m_pCoordSys)
        {
        m_pCoordSys = pi_rpCoordSys;
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
double HGFTolerance::GetLinearTolerance
(
) const
    {
    if (m_Tolerance < 0.0)
        {
        double x0, y0, x1, y1, x2, y2, x3, y3;

        m_Quadrilateral.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

        m_Tolerance = MAX (sqrt(SQUARE(x2 - x0) + SQUARE(y2 - y0)), sqrt(SQUARE(x3 - x1) + SQUARE(y3 - y1)));
        }

    return m_Tolerance;
    }

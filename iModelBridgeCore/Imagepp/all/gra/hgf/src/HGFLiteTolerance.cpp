//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLiteTolerance.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFLiteTolerance
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFLiteTolerance.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>


#define SQUARE(a) ((a)*(a))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance::HGFLiteTolerance ()
    : m_XMin(0.0), 
      m_YMin(0.0), 
      m_Quadrilateral(), 
      m_Tolerance(-1.0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance::HGFLiteTolerance
(
    double pi_x0,
    double pi_y0,
    double pi_x1,
    double pi_y1,
    double pi_x2,
    double pi_y2,
    double pi_x3,
    double pi_y3
)
    : m_XMin(MIN(MIN(pi_x0, pi_x1), MIN(pi_x2, pi_x3))),
      m_YMin(MIN(MIN(pi_y0, pi_y1), MIN(pi_y2, pi_y3))),
      m_Quadrilateral(pi_x0 - m_XMin, pi_y0 - m_YMin,
                      pi_x1 - m_XMin, pi_y1 - m_YMin,
                      pi_x2 - m_XMin, pi_y2 - m_YMin,
                      pi_x3 - m_XMin, pi_y3 - m_YMin),
      m_Tolerance (-1.0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance::HGFLiteTolerance
(
    double pi_XMin,
    double pi_YMin,
    double pi_XMax,
    double pi_YMax
)
    : m_XMin(pi_XMin),
      m_YMin(pi_YMin),
      m_Quadrilateral(pi_XMin - m_XMin, pi_YMin - m_YMin,
                      pi_XMin - m_XMin, pi_YMax - m_YMin,
                      pi_XMax - m_XMin, pi_YMax - m_YMin,
                      pi_XMax - m_XMin, pi_YMin - m_YMin),
      m_Tolerance (-1.0)
    {
    HASSERT(pi_XMin < pi_XMax);
    HASSERT(pi_YMin < pi_YMax);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance::HGFLiteTolerance
(
    const HGFLiteTolerance& pi_rObj
)
    {
    m_Quadrilateral = pi_rObj.m_Quadrilateral;
    m_XMin = pi_rObj.m_XMin;
    m_YMin = pi_rObj.m_YMin;
    m_Tolerance = pi_rObj.m_Tolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance::~HGFLiteTolerance
(
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HGFLiteTolerance& HGFLiteTolerance::operator=
(
    const HGFLiteTolerance& pi_rObj
)
    {
    if (this != &pi_rObj)
        {
        m_Quadrilateral = pi_rObj.m_Quadrilateral;
        m_XMin = pi_rObj.m_XMin;
        m_YMin = pi_rObj.m_YMin;
        m_Tolerance = pi_rObj.m_Tolerance;
        }

    return *this;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
double HGFLiteTolerance::GetLinearTolerance
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



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGFLiteTolerance::TransformDirect(const HGF2DTransfoModel& pi_rModel)
    {
    double x0, y0, x1, y1, x2, y2, x3, y3;

    m_Quadrilateral.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

    HGF2DPosition Point0(x0 + m_XMin, y0 + m_YMin);
    HGF2DPosition Point1(x1 + m_XMin, y1 + m_YMin);
    HGF2DPosition Point2(x2 + m_XMin, y2 + m_YMin);
    HGF2DPosition Point3(x3 + m_XMin, y3 + m_YMin);

    pi_rModel.ConvertPosDirect(&Point0);
    pi_rModel.ConvertPosDirect(&Point1);
    pi_rModel.ConvertPosDirect(&Point2);
    pi_rModel.ConvertPosDirect(&Point3);

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

    m_Tolerance = -1.0;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGFLiteTolerance::TransformInverse(const HGF2DTransfoModel& pi_rModel)
    {
    double x0, y0, x1, y1, x2, y2, x3, y3;

    m_Quadrilateral.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

    HGF2DPosition Point0(x0 + m_XMin, y0 + m_YMin);
    HGF2DPosition Point1(x1 + m_XMin, y1 + m_YMin);
    HGF2DPosition Point2(x2 + m_XMin, y2 + m_YMin);
    HGF2DPosition Point3(x3 + m_XMin, y3 + m_YMin);

    pi_rModel.ConvertPosInverse(&Point0);
    pi_rModel.ConvertPosInverse(&Point1);
    pi_rModel.ConvertPosInverse(&Point2);
    pi_rModel.ConvertPosInverse(&Point3);

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

    m_Tolerance = -1.0;

    return true;
    }

//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLiteTolerance.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFLiteTolerance
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGFLiteTolerance.h>


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
    : m_XMin(min(min(pi_x0, pi_x1), min(pi_x2, pi_x3))),
      m_YMin(min(min(pi_y0, pi_y1), min(pi_y2, pi_y3))),
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

        m_Tolerance = max (sqrt(SQUARE(x2 - x0) + SQUARE(y2 - y0)), sqrt(SQUARE(x3 - x1) + SQUARE(y3 - y1)));
        }

    return m_Tolerance;
    }

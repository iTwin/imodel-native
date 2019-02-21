//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE3DPlane.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "hstdcpp.h"
#include "HVE3DPlane.h"


/**----------------------------------------------------------------------------
 Default constructor.

 The default plane is perpendicular to Z axis.

-----------------------------------------------------------------------------*/
HVE3DPlane::HVE3DPlane()
    : m_FirstPoint(0.0, 0.0, 0.0),
      m_SecondPoint(10.0, 0.0, 0.0),
      m_ThirdPoint(0.0, 10.0, 0.0)
    {
    // Compute plane equation parameter
    PrepareEquation();
    }



/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Plane to duplicate.
-----------------------------------------------------------------------------*/
HVE3DPlane::HVE3DPlane(const HVE3DPlane& pi_rObj)
    : m_FirstPoint(pi_rObj.m_FirstPoint),
      m_SecondPoint(pi_rObj.m_SecondPoint),
      m_ThirdPoint(pi_rObj.m_ThirdPoint)
    {
    // Compute plane equation parameter
    PrepareEquation();
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying three points. The three points provided
 must not be co-linear, nor define a plane parallel to the Z axis.

 @param pi_rFirstPoint A HGF3DPoint specifying the first point of the plane
                       definition.

 @param pi_rSecondPoint A HGF3DPoint specifying the second point of the plane
                        definition.

 @param pi_rThirdPoint A HGF3DPoint specifying the third point of the plane
                       definition.


-----------------------------------------------------------------------------*/
HVE3DPlane::HVE3DPlane(const HGF3DPoint& pi_rFirstPoint,
                       const HGF3DPoint& pi_rSecondPoint,
                       const HGF3DPoint& pi_rThirdPoint)
    {
    // Make sure that the points are not colinear

    // Make sure the plane is not parallel to Z axis.

    // Copy points
    m_FirstPoint = pi_rFirstPoint;
    m_SecondPoint = pi_rSecondPoint;
    m_ThirdPoint = pi_rThirdPoint;

    // Compute plane equation parameter
    PrepareEquation();
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HVE3DPlane::~HVE3DPlane()
    {
    }

/**----------------------------------------------------------------------------
  Assignment operator

  It duplicates a plane
  @param pi_rObj The plane to duplicate

  @return a reference to self to be used as a l-value.
-----------------------------------------------------------------------------*/
HVE3DPlane& HVE3DPlane::operator=(const HVE3DPlane& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        m_FirstPoint = pi_rObj.m_FirstPoint;
        m_SecondPoint = pi_rObj.m_SecondPoint;
        m_ThirdPoint = pi_rObj.m_ThirdPoint;

        m_A = pi_rObj.m_A;
        m_B = pi_rObj.m_B;
        m_C = pi_rObj.m_C;
        m_D = pi_rObj.m_D;
        }

    return(*this);
    }






/**----------------------------------------------------------------------------
 PRIVATE METHOD
 PrepareEquation

  This function uses the internal points to prepare the plane equation parameter


-----------------------------------------------------------------------------*/
void HVE3DPlane::PrepareEquation()
    {
    // Plane Equation : Ax + By + Cz + D = 0
    double Dx1 = m_FirstPoint.GetX() - m_ThirdPoint.GetX();
    double Dy1 = m_FirstPoint.GetY() - m_ThirdPoint.GetY();
    double Dz1 = m_FirstPoint.GetZ() - m_ThirdPoint.GetZ();

    double Dx2 = m_SecondPoint.GetX() - m_ThirdPoint.GetX();
    double Dy2 = m_SecondPoint.GetY() - m_ThirdPoint.GetY();
    double Dz2 = m_SecondPoint.GetZ() - m_ThirdPoint.GetZ();

    m_A = Dy1 * Dz2 - Dz1 * Dy2;
    m_B = -Dx1 * Dz2 + Dz1 * Dx2;
    m_C = Dx1 * Dy2 - Dy1 * Dx2;
    m_D = -m_A * m_FirstPoint.GetX() - m_B * m_FirstPoint.GetY() - m_C * m_FirstPoint.GetZ();

// Replaced by IsValid() setting
//    HASSERT(m_C != 0);

    }



//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DPlane.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE3DPlane
//-----------------------------------------------------------------------------

#pragma once

#include "HGF3DPoint.h"
#include "HGF2DPosition.h"
// HPM_DECLARE_HEADER(HVE3DPlane)

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a 3D plane into space. A plane is defined using 3 points
    located into 3D space, or three 2D points to which are assigned Z (elevation).
    The three points may not be colinear, and in the current implementation, the
    plane may not be parallel to the Z axis.
    -----------------------------------------------------------------------------
*/
class HVE3DPlane
    {

public:

    // Primary methods
    HVE3DPlane();
    HVE3DPlane(const HVE3DPlane& pi_rObject);
    HVE3DPlane(const HGF3DPoint& pi_rFirstPoint,
               const HGF3DPoint& pi_rSecondPoint,
               const HGF3DPoint& pi_rThirdPoint);
    virtual            ~HVE3DPlane();

    HVE3DPlane&        operator=(const HVE3DPlane& pi_rObj);

    // Boolean compare operations
    bool               operator==(const HVE3DPlane& pi_rObj) const;
    bool               operator!=(const HVE3DPlane& pi_rObj) const;


    // Plane specific interface

    // Elevation extraction
    double            GetElevationAt(const HGF2DPosition& pi_rPoint) const;

    const HGF3DPoint&  GetFirstDefinitionPoint() const;
    const HGF3DPoint&  GetSecondDefinitionPoint() const;
    const HGF3DPoint&  GetThirdDefinitionPoint() const;

    bool               IsValid() const;


private:

    void PrepareEquation();


    HGF3DPoint m_FirstPoint;
    HGF3DPoint m_SecondPoint;
    HGF3DPoint m_ThirdPoint;


    // The equation of a plane is Ax+By+Cz+D = 0
    double m_A;
    double m_B;
    double m_C;
    double m_D;

    };
END_IMAGEPP_NAMESPACE

#include "HVE3DPlane.hpp"


//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteLine.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLiteLine
//-----------------------------------------------------------------------------
// Description of an angle. An angle is a quantity with angular
// units.
//-----------------------------------------------------------------------------

#pragma once

#include "HGFBearing.h"
#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert


    This class implements a line. This is a quite simple class defining a line infinite in both directions.
    The line can be defined in many ways. Either by a position and a bearing, two points, or mathematical
    parameters of the line equation (Slope and Intercept). The class implements methods to perform some
    mathematical operations upon lines and location (HGF2DPosition). Such operations include finding
    a crossing point between lines, and ways to obtains the closest point on line from a specified
    location. A line does not have a specific direction. For this reason two bearings can be used
    when defining a line by bearing. It follows that two bearing could be returned by the
    CalculateBearing method. Either one would be valid.

    The equation of a line is :

    y = Mx + B

    where :
    M is the slope
    B is the intercept with the Y axis.

    If the slope M is infinity (the line is vertical), then the equation reduces to :

    where G is then the intercept with the X axis.

    Due to its nature, the line will sometimes refuse to interact with other lines, if the coordinate
    systems between these lines is different and not related through a transformation which preserves
    linearity. If a transformation model did not preserve linearity was accepted, then the line,
    when transformed in the other coordinate system, would result in an
    infinite size object which would not be a line.

    The class is coded to maximize precision. This includes the internal representation of lines in
    the inverse form when the slope becomes too high

    x = Ny + C
    where:
    N is the invert slope
    C in the intercept with the X axis.

    Externally to the class however, the line is always available in the first form.
    -----------------------------------------------------------------------------
*/
class HGF2DLiteLine
    {
public:

    enum CrossState
        {
        CROSS_FOUND,
        PARALLEL
        };

    // Primary methods
    HGF2DLiteLine (const HGF2DPosition& pi_rFirstPoint,
                   const HGF2DPosition& pi_rSecondPoint);
    HGF2DLiteLine (const HGF2DPosition& pi_rRefPoint,
                   const HGFBearing&    pi_rBearing);
    HGF2DLiteLine (double               pi_Slope,
                   double               pi_rIntercept);
    HGF2DLiteLine (const HGF2DLiteLine&    pi_rObject);
    virtual         ~HGF2DLiteLine();

    HGF2DLiteLine&      operator=(const HGF2DLiteLine& pi_rObj);

    // Compare operators
    bool           operator== (const HGF2DLiteLine& pi_rObject) const;
    bool           operator!= (const HGF2DLiteLine& pi_rObject) const;

    // Compare operators
    bool           IsEqualTo(const HGF2DLiteLine& pi_rObject) const;
    bool           IsEqualTo(const HGF2DLiteLine& pi_rObject, double pi_Epsilon) const;
    bool           IsEqualToAutoEpsilon(const HGF2DLiteLine& pi_rObject) const;


    // Operations on lines
    HGFBearing          CalculateBearing() const;
    HGF2DPosition       CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const;
    double              CalculateShortestDistance(const HGF2DPosition& pi_rPoint) const;
    HGF2DLiteLine::CrossState     IntersectLine(const HGF2DLiteLine& pi_rLine, HGF2DPosition* po_pPoint) const;
    double              GetIntercept() const;
    double              GetSlope () const;
    bool                IsParallelTo(const HGF2DLiteLine& pi_rLine) const;
    bool                IsVertical() const;


protected:

private:

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
//            HASSERT(m_Slope < 2.0);
        }
#endif

    double         m_Intercept;
    double         m_Slope;
    bool           m_InvertSlope;
    };
END_IMAGEPP_NAMESPACE

#include "HGF2DLiteLine.hpp"

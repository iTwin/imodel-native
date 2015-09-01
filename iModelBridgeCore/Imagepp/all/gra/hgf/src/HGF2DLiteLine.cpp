//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLiteLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DLiteLine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGF2DLiteLine.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>

/** -----------------------------------------------------------------------------
    Constructor by bearing

    The line is constructed from a point and bearing. The interpretation coordinate
    system is copied from the point.

    @param pi_rRefPoint IN A constant reference to an HGF2DPosition by which the
                           line passes.

    @param pi_rBearing IN The bearing from pi_rRefPoint indicating the direction
                          of the line.

    @code
        HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
        HGF2DLiteLine               MyLine (MyFirstPoint,
                                        HGFBearing(PI/4)));
    @end
    -----------------------------------------------------------------------------
*/
HGF2DLiteLine::HGF2DLiteLine(const HGF2DPosition& pi_rRefPoint,
                             const HGFBearing&    pi_rBearing)
    : m_Slope(0.0),
//  m_Vertical(false),
      m_InvertSlope(false)
    {
    // Create a displacement (with an arbitrary distance)
    HGF2DDisplacement   MyDisplacement;
    MyDisplacement.SetByBearing (pi_rBearing, 1000.0);

    // Check that the line has not a steep slope
    if (MyDisplacement.GetDeltaX() == 0.0 && MyDisplacement.GetDeltaY() == 0.0)
        {

        // We have a null segment ... give it any parameters
        m_InvertSlope = true;

        // Compute slope
        m_Slope = 0.0;

        // Compute intercept
        m_Intercept = 0.0;

        }
    else if (fabs(MyDisplacement.GetDeltaX()) > fabs(MyDisplacement.GetDeltaY()))
        {
        // Line has an ordinary slope
        m_InvertSlope = false;

        // Compute slope
        m_Slope = (MyDisplacement.GetDeltaY() / MyDisplacement.GetDeltaX());

        // Compute intercept
        m_Intercept = pi_rRefPoint.GetY() - (pi_rRefPoint.GetX() * m_Slope);

        }
    else
        {
        // Line has a high slope ... we will invert parameters
        m_InvertSlope = true;

        // Compute slope
        m_Slope = (MyDisplacement.GetDeltaX() / MyDisplacement.GetDeltaY());

        // Compute intercept
        m_Intercept = pi_rRefPoint.GetX() - (pi_rRefPoint.GetY() * m_Slope);
        }

    HINVARIANTS;
    }

/** -----------------------------------------------------------------------------
    Constructor by two points

    The line is constructed from the two provided points which may not be
    identical. The coordinate system assigned to the line is then the coordinate
    system of the first point.

    @param pi_rFirstPoint IN Reference to an HGF2DPosition object containing the
                             definition of the first alignement point of the line.
                             From this point is also copied the smart pointer to
                             the coordinate system which will also be used in the
                             interpretation of the line.

    @param pi_rSecondPoint IN Reference to an HGF2DPosition object containing the
                            definition of the second alignement point of the line.


    @code
        HFCPtr<HGF2DCoordSys>   pMyWorld(new HGF2DCoordSys());
        HGF2DPosition           MyFirstPoint(10, 10);
        HGF2DPosition           MySecondPoint(15, 16)
        HGF2DLiteLine           MyLine(MyFirstPoint, MySecondPoint);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DLiteLine::HGF2DLiteLine(const HGF2DPosition& pi_rFirstPoint,
                             const HGF2DPosition& pi_rSecondPoint)
    : m_Slope(0.0),
//  m_Vertical(false),
      m_InvertSlope(false)
    {
    // Compute displacement
    HGF2DDisplacement   MyDisplacement(pi_rFirstPoint-pi_rSecondPoint);

    // Check for infity displacements
    if (!BeNumerical::BeFinite(MyDisplacement.GetDeltaX()) && !BeNumerical::BeFinite(MyDisplacement.GetDeltaY()))
        {
        // Both deltas are infinity ... slope would be undefined

        // We assume perfectly horizontal or vertical lines

        // Check if X coordinates are both positive
        if (pi_rFirstPoint.GetX() > 0 && pi_rSecondPoint.GetX() > 0)
            {
            // Both X coordinates are of the same sign ... check Y
            if (pi_rFirstPoint.GetY() > 0 && pi_rSecondPoint.GetY() > 0)
                {
                // Both Y are also of the same sign ... we do not know what to do
                // We set the slope to 1 with intercept at 0.0
                m_InvertSlope = false;
                m_Intercept = 0.0;
                m_Slope = 1.0;
                }
            else
                {
                // Y signs are different .... horizontal line
                m_InvertSlope = false;
                m_Slope = 0.0;
                m_Intercept = pi_rFirstPoint.GetY();
                }
            }
        else
            {
            // The X coordinates are reversed sign ... check Y
            if (pi_rFirstPoint.GetY() > 0 && pi_rSecondPoint.GetY() > 0)
                {
                // Y coordinate are same sign ... vertical line
                m_InvertSlope = true;
                m_Slope = 0.0;
                m_Intercept = pi_rFirstPoint.GetX();
                }
            else
                {
                // Ys are different sign ... we do not know what to do
                // We set the slope to 1 with intercept at 0.0
                m_InvertSlope = false;
                m_Intercept = 0.0;
                m_Slope = 1.0;
                }
            }
        }
    else
        {

        // Check that the line is not vertical
        if (MyDisplacement.GetDeltaX() == 0.0 && MyDisplacement.GetDeltaY() == 0.0)
            {
            // We have a null segment ... give it any parameters
            m_InvertSlope = true;

            // Compute slope
            m_Slope = 0.0;

            // Compute intercept
            m_Intercept = 0.0;

            }
        else if (fabs(MyDisplacement.GetDeltaX()) > fabs(MyDisplacement.GetDeltaY()))
            {
            // Line has an ordinary slope
            m_InvertSlope = false;

            // Compute slope
            m_Slope = (MyDisplacement.GetDeltaY() / MyDisplacement.GetDeltaX());

            // Compute intercept
            m_Intercept = pi_rFirstPoint.GetY() - (pi_rFirstPoint.GetX() * m_Slope);

            }
        else
            {
            // Line has a high slope ... we will invert parameters
            m_InvertSlope = true;

            // Compute slope
            m_Slope = (MyDisplacement.GetDeltaX() / MyDisplacement.GetDeltaY());

            // Compute intercept
            m_Intercept = pi_rFirstPoint.GetX() - (pi_rFirstPoint.GetY() * m_Slope);
            }
        }

    HINVARIANTS;

    }


/** -----------------------------------------------------------------------------
    Equality operator

    Equality compare operator. It returns true if the lines are equal, and 0
    otherwise. For two lines to be equal, they must represent the same line
    (same slope and intercept) when expressed in the left operand coordinate
    system. In all other case they are different. If the transformation model
    expressing the relation between the coordinate systems of each line does
    not preserve linearity, then false is systematically returned.

    @param pi_rObj Constant reference to a HGF2DLiteLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator!=()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteLine::operator==(const HGF2DLiteLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // They do have the same coordinate system ... compare members
    Result = (m_Intercept == pi_rObj.m_Intercept) &&
             (m_InvertSlope == pi_rObj.m_InvertSlope) &&
             (m_Slope == pi_rObj.m_Slope);

    return(Result);
    }


/** -----------------------------------------------------------------------------
    Equality compare operator with epsilon (tolerance) application.
    It reutns true if the lines are equal, and false otherwise.
    For two lines to be equal, they must represent the same line
    (same slope and intercept) within the fixed, tolerance when expressed in the
    left operand coordinate system. In all other case they are different. If the
    transformation model expressing the relation between the coordinate systems
    of each line does not preserve linearity, then false is systematically returned.

    @param pi_rObj Constant reference to a HGF2DLiteLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteLine::IsEqualTo(const HGF2DLiteLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // They do have the same coordinate system ... compare members
    Result = (HDOUBLE_EQUAL_EPSILON(m_Intercept, pi_rObj.m_Intercept)) &&
             (
                 (
                     m_InvertSlope == pi_rObj.m_InvertSlope &&
                     HDOUBLE_EQUAL_EPSILON(m_Slope, pi_rObj.m_Slope)
                 ) ||
                 (
                     m_InvertSlope != pi_rObj.m_InvertSlope &&
                     pi_rObj.m_Slope != 0.0 &&
                     HDOUBLE_EQUAL_EPSILON(m_Slope, 1.0 / pi_rObj.m_Slope)
                 )
             );

    return(Result);
    }

/** -----------------------------------------------------------------------------
    Equality compare operator with epsilon (tolerance) application.
    It reutns true if the lines are equal, and false otherwise.
    For two lines to be equal, they must represent the same line
    (same slope and intercept) within the specified tolerance when expressed in the
    left operand coordinate system. In all other case they are different. If the
    transformation model expressing the relation between the coordinate systems
    of each line does not preserve linearity, then false is systematically returned.

    @param pi_rObj Constant reference to a HGF2DLiteLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteLine::IsEqualTo(const HGF2DLiteLine& pi_rObj, double pi_Epsilon) const
    {
    HINVARIANTS;

    bool   Result;

    // They do have the same coordinate system ... compare members
    Result = (HDOUBLE_EQUAL(m_Intercept, pi_rObj.m_Intercept, pi_Epsilon)) &&
             (
                 (
                     m_InvertSlope == pi_rObj.m_InvertSlope &&
                     HDOUBLE_EQUAL(m_Slope, pi_rObj.m_Slope, pi_Epsilon)
                 ) ||
                 (
                     m_InvertSlope != pi_rObj.m_InvertSlope &&
                     pi_rObj.m_Slope != 0.0 &&
                     HDOUBLE_EQUAL(m_Slope, 1.0 / pi_rObj.m_Slope, pi_Epsilon)
                 )
             );


    return(Result);
    }

/** -----------------------------------------------------------------------------
    Equality compare operator with epsilon (tolerance) application.
    It reutns true if the lines are equal, and false otherwise.
    For two lines to be equal, they must represent the same line
    (same slope and intercept) within the determined tolerance when expressed in the
    left operand coordinate system. In all other case they are different. If the
    transformation model expressing the relation between the coordinate systems
    of each line does not preserve linearity, then false is systematically returned.

    @param pi_rObj Constant reference to a HGF2DLiteLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteLine::IsEqualToAutoEpsilon(const HGF2DLiteLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // They do have the same coordinate system ... compare members
    Result = (HDOUBLE_EQUAL(m_Intercept, pi_rObj.m_Intercept, fabs(HGLOBAL_EPSILON * m_Intercept))) &&
             (
                 (
                     m_InvertSlope == pi_rObj.m_InvertSlope &&
                     HDOUBLE_EQUAL(m_Slope, pi_rObj.m_Slope, fabs(HGLOBAL_EPSILON * m_Slope))
                 ) ||
                 (   m_InvertSlope != pi_rObj.m_InvertSlope &&
                     pi_rObj.m_Slope != 0.0 &&
                     HDOUBLE_EQUAL(m_Slope, 1.0 / pi_rObj.m_Slope, fabs(HGLOBAL_EPSILON * m_Slope))
                 )
             );

    return(Result);
    }

/** -----------------------------------------------------------------------------
    These methods return true if the lines are parallel one to the other.
    If the transformation model expressing the relation between the coordinate
    systems of each line does not preserve linearity, then false is systematically
    returned.

    @param pi_rObj Constant reference to the line to check if it is parallel to self.

    @return true if line is parallel to other line. false otherwise. Note that
            false will be systematicaly returned if the transformation model
            expressing the relation between the coordinate systems of each line
            does not preserve linearity.

    @see IntersectLine()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteLine::IsParallelTo (const HGF2DLiteLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;
    // They do have the same coordinate system ... compare members
    Result = (
                 (
                     m_InvertSlope == pi_rObj.m_InvertSlope &&
                     HDOUBLE_EQUAL_EPSILON(m_Slope, pi_rObj.m_Slope)
                 ) ||
                 (
                     m_InvertSlope != pi_rObj.m_InvertSlope &&
                     pi_rObj.m_Slope != 0.0 &&
                     HDOUBLE_EQUAL_EPSILON(m_Slope, 1.0 / pi_rObj.m_Slope)
                 )
             );

    return (Result);
    }



/** -----------------------------------------------------------------------------
    This method returns the bearing of the line. Two bearings could be returned
    by the method, and the bearing returned is either one of them.

    @return The bearing of the line
    -----------------------------------------------------------------------------
*/
HGFBearing HGF2DLiteLine::CalculateBearing() const
    {
    HINVARIANTS;

    HGFBearing ReturnBearing;

    // Check if slope is inverted
    if (m_InvertSlope)
        {
        // The slope is inverted ...
        // Check if slope is null
        if (m_Slope == 0.0)
            {
            ReturnBearing.SetAngle(PI / 2);
            }
        else
            {
            ReturnBearing = HGF2DDisplacement(1.0, 1.0 / m_Slope).CalculateBearing();
            }
        }
    else
        {
        ReturnBearing = HGF2DDisplacement(1.0, m_Slope).CalculateBearing();
        }

    return(ReturnBearing);
    }


/** -----------------------------------------------------------------------------
    This method calculates and returns the closest point on line from the given
    point to the line. This closest point is the point which is on the line
    forming a perpendicular with the given point.

    The point returned is interpreted in the same coordinate system as the line.

    @param pi_rPoint IN Constant reference to the point to find shortest distance to
                        line of.

    @return The closest point found.
    -----------------------------------------------------------------------------
*/
HGF2DPosition HGF2DLiteLine::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    HINVARIANTS;

    HGF2DPosition       ReturnPoint;


    HGF2DPosition APoint;

    double DeltaX;
    double DeltaY;

    if (m_InvertSlope)
        {
        DeltaX = m_Slope;
        DeltaY = 1.0;
        APoint.SetX(m_Intercept);
        APoint.SetY(0.0);
        }
    else
        {
        DeltaX = 1.0;
        DeltaY = m_Slope;
        APoint.SetX(0.0);
        APoint.SetY(m_Intercept);
        }

    double SlopeSqLen = DeltaX * DeltaX + DeltaY * DeltaY;
    double InvSlopeSqLen = 1.0 / SlopeSqLen;

    double TempPointX = pi_rPoint.GetX();
    double TempPointY = pi_rPoint.GetY();

    double APointX = APoint.GetX();
    double APointY = APoint.GetY();


    double k2 =    ( DeltaX * (TempPointY - APointY) + DeltaY * (APointX - TempPointX) ) * -InvSlopeSqLen;

    double ResultX = TempPointX - k2 * DeltaY;
    double ResultY = TempPointY + k2 * DeltaX;

    ReturnPoint.SetX(ResultX);
    ReturnPoint.SetY(ResultY);


    return(ReturnPoint);

    }

/** -----------------------------------------------------------------------------
    This method calculates and returns the shortest distance to line from the
    given point. This shortest distance is the distance which is on the line
    forming a perpendicular with the given point. This method is much more
    performant than the CalculateClosestPoint() method and should be used if
    only the distance is needed.

    The distance returned is interpreted in the X unit of the coordinate
    system of the line.

    @param pi_rPoint IN Constant reference to the point to find closest point on
                        line of.

    @return Shortest distance to the line.
    -----------------------------------------------------------------------------
*/
double HGF2DLiteLine::CalculateShortestDistance(const HGF2DPosition& pi_rPoint) const
    {
    HINVARIANTS;


    double         ReturnValue;
    // Convert line description to Ax + BY + C
    // formula is mx -y + b
    // A = m
    // B = -1
    // C = b
    // and shortest distance is Ax + By + C / sqrt(A^2 + B^2)
    // which is mx - y + b / sqrt(m^2 + 1)

    if (m_InvertSlope)
        ReturnValue =  fabs((m_Slope * pi_rPoint.GetY() - pi_rPoint.GetX() + m_Intercept) / sqrt(m_Slope * m_Slope + 1));
    else
        ReturnValue =  fabs((m_Slope * pi_rPoint.GetX() - pi_rPoint.GetY() + m_Intercept) / sqrt(m_Slope * m_Slope + 1));

    return(ReturnValue);

    }



/** -----------------------------------------------------------------------------
    This method calculates and returns the crossing point with the given
    line. The transformation model expressing the relation between the coordinate
    system of each line must preserve linearity.

    It is possible that no crossing point exists. In that case, the state of
    the returned point is undefined, and the returned state indicates if crossing
    could be performed. The only reason possible for two line not to cross is
    if they are parallel one to the other. Even if the line are one on top of
    the other, parallel lines do not cross by definition.

    The point returned in interpreted in the same coordinate system as the line.

    @param pi_rLine IN Constant reference to the HGF2DLiteLine to find crossing point with.

    @param po_pPoint OUT Pointer to HGF2DPosition that receives the crossing point
                         if there is one.

    @return The status of the crossing operation. This status is
            HGF2DLiteLine::CROSS_FOUND if a crossing point is found,
            HGF2DLiteLine::PARALLEL if the line is parallel to the line given.
            Note that the value of the HGF2DPosition pointed to by po_pPoint is
            undefined if the returned value is different from HGF2DLiteLine::CROSS_FOUND.
    -----------------------------------------------------------------------------
*/
HGF2DLiteLine::CrossState HGF2DLiteLine::IntersectLine(const HGF2DLiteLine& pi_rLine, HGF2DPosition* po_pPoint) const
    {
    HINVARIANTS;


    HPRECONDITION(po_pPoint != 0);

    CrossState  TheState;

    // Check if the two line share the same coordinate system
    TheState = (
                   (
                       m_InvertSlope == pi_rLine.m_InvertSlope &&
                       HDOUBLE_EQUAL(m_Slope, pi_rLine.m_Slope, HGLOBAL_EPSILON / 100.0)
                   ) ||
                   (
                       m_InvertSlope != pi_rLine.m_InvertSlope &&
                       pi_rLine.m_Slope != 0.0 &&
                       HDOUBLE_EQUAL(m_Slope, 1.0 / pi_rLine.m_Slope, HGLOBAL_EPSILON / 100.0)
                   )
                   ?
                   PARALLEL : CROSS_FOUND
               );


    if (TheState == CROSS_FOUND)
        {
        // Check if first line is vertical
        if (m_InvertSlope && pi_rLine.m_InvertSlope)
            {
            // The slope of self and given are inverted
            // The intersection of two lines is then found by :
            //     b2 - b1
            // Y = -------
            //     m1 - m2
            //
            // x = m1*y + b1
            po_pPoint->SetY((pi_rLine.m_Intercept - m_Intercept) / (m_Slope - pi_rLine.m_Slope));
            po_pPoint->SetX((m_Slope * po_pPoint->GetY()) + m_Intercept);

            }
        else if (!m_InvertSlope && !pi_rLine.m_InvertSlope)
            {
            // None of the slope of the lines are inverted
            // The intersection of two lines is found by :
            //     b2 - b1
            // x = -------
            //     m1 - m2
            //
            // y = m1*x + b1
            //
            po_pPoint->SetX((pi_rLine.m_Intercept - m_Intercept) / (m_Slope - pi_rLine.m_Slope));
            po_pPoint->SetY((m_Slope * po_pPoint->GetX()) + m_Intercept);
            }
        else
            {
            // What we have is one line with slope inverted and the other not inverted
            // we will reverse parameters for the one having the inverted slope
            if (m_InvertSlope)
                {
                // Check if this slope is 0.0 (vertical line)
                if (m_Slope == 0.0)
                    {
                    // Self is vertical ...
                    po_pPoint->SetX(m_Intercept);
                    po_pPoint->SetY(pi_rLine.m_Slope * m_Intercept + pi_rLine.m_Intercept);
                    }
                else
                    {
                    // Self has an inverted slope ... reverse it
                    double NewSlope = 1.0 / m_Slope;
                    double NewIntercept = -m_Intercept / m_Slope;

                    po_pPoint->SetX((pi_rLine.m_Intercept - NewIntercept) / (NewSlope - pi_rLine.m_Slope));
                    po_pPoint->SetY((pi_rLine.m_Slope * po_pPoint->GetX()) + pi_rLine.m_Intercept);
                    }
                }
            else
                {
                // Check if this slope is 0.0 (vertical line)
                if (pi_rLine.m_Slope == 0.0)
                    {
                    // Given is vertical ...
                    po_pPoint->SetX(pi_rLine.m_Intercept);
                    po_pPoint->SetY(m_Slope * pi_rLine.m_Intercept + m_Intercept);
                    }
                else
                    {
                    // Self has an inverted slope ... reverse it
                    double NewSlope = 1.0 / pi_rLine.m_Slope;
                    double NewIntercept = -pi_rLine.m_Intercept / pi_rLine.m_Slope;

                    po_pPoint->SetX((NewIntercept - m_Intercept) / (m_Slope - NewSlope));
                    po_pPoint->SetY((m_Slope * po_pPoint->GetX()) + m_Intercept);
                    }

                }
            }
        }
    return (TheState);
    }



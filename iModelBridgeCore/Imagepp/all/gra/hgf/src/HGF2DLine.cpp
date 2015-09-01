//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DLine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGF2DLine.h>

/** -----------------------------------------------------------------------------
    Constructor by bearing

    The line is constructed from a point and bearing. The interpretation coordinate
    system is copied from the point.

    @param pi_rRefPoint IN A constant reference to an HGF2DLocation by which the
                           line passes.

    @param pi_rBearing IN The bearing from pi_rRefPoint indicating the direction
                          of the line.

    @code
        HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
        HGF2DLine               MyLine (MyFirstPoint,
                                        HGFBearing(PI/4));
    @end
    -----------------------------------------------------------------------------
*/
HGF2DLine::HGF2DLine(const HGF2DLocation& pi_rRefPoint,
                     const HGFBearing&    pi_rBearing)
    : m_Slope(0.0),
//  m_Vertical(false),
      m_InvertSlope(false),
      m_pCoordSys(pi_rRefPoint.GetCoordSys())
    {
    // Create a displacement (with an arbitrary distance)
    HGF2DDisplacement   MyDisplacement(pi_rBearing, 1000.0);

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

    @param pi_rFirstPoint IN Reference to an HGF2DLocation object containing the
                             definition of the first alignement point of the line.
                             From this point is also copied the smart pointer to
                             the coordinate system which will also be used in the
                             interpretation of the line.

    @param pi_rSecondPoint IN Reference to an HGF2DLocation object containing the
                            definition of the second alignement point of the line.


    @code
        HFCPtr<HGF2DCoordSys>   pMyWorld(new HGF2DCoordSys());
        HGF2DLocation           MyFirstPoint(10, 10, pMyWorld);
        HGF2DLocation           MySecondPoint(16, 12, pMyWorld);
        HGF2DLine               MyLine(MyFirstPoint, MySecondPoint);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DLine::HGF2DLine(const HGF2DLocation& pi_rFirstPoint,
                     const HGF2DLocation& pi_rSecondPoint)
    : m_Slope(0.0),
//  m_Vertical(false),
      m_InvertSlope(false),
      m_pCoordSys(pi_rFirstPoint.GetCoordSys())
    {
    // Compute displacement
    HGF2DDisplacement   MyDisplacement(pi_rFirstPoint-pi_rSecondPoint);

    // Check for infity displacements
    if (!BeNumerical::BeFinite(MyDisplacement.GetDeltaX()) && !BeNumerical::BeFinite(MyDisplacement.GetDeltaY()))
        {
        // Both deltas are infinity ... slope would be undefined

        // We assume perfectly horizontal or vertical lines

        // We obtain second coordinates in the first coordinate system
        HGF2DLocation   SecondPoint(pi_rSecondPoint, pi_rFirstPoint.GetCoordSys());

        // Check if X coordinates are both positive
        if (pi_rFirstPoint.GetX() > 0 && SecondPoint.GetX() > 0)
            {
            // Both X coordinates are of the same sign ... check Y
            if (pi_rFirstPoint.GetY() > 0 && SecondPoint.GetY() > 0)
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
            if (pi_rFirstPoint.GetY() > 0 && SecondPoint.GetY() > 0)
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

    @param pi_rObj Constant reference to a HGF2DLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator!=()
    -----------------------------------------------------------------------------
*/
bool HGF2DLine::operator==(const HGF2DLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // Two lines to be compared must be related by a linearity
    // preserving model
    if ((m_pCoordSys != pi_rObj.m_pCoordSys) &&
        (!m_pCoordSys->GetTransfoModelTo(pi_rObj.m_pCoordSys)->PreservesLinearity()))
        {
        Result = false;
        }
    else
        {
        // Check if the two line share the same coordinate system
        if (m_pCoordSys == pi_rObj.m_pCoordSys)
            {
            // They do have the same coordinate system ... compare members
            Result = (m_Intercept == pi_rObj.m_Intercept) &&
                     (m_InvertSlope == pi_rObj.m_InvertSlope) &&
                     (m_Slope == pi_rObj.m_Slope);
            }
        else
            {
            // They do not share the same coordinate system ... create copy
            HGF2DLine   TempLine(pi_rObj);

            // Change coordinate system of copy
            TempLine.ChangeCoordSys(m_pCoordSys);

            // Compare mathematical member attributes
            Result = (m_Intercept == TempLine.m_Intercept) &&
                     (m_InvertSlope == TempLine.m_InvertSlope) &&
                     (m_Slope == TempLine.m_Slope);
            }
        }

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

    @param pi_rObj Constant reference to a HGF2DLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLine::IsEqualTo(const HGF2DLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // Two lines to be compared must be related by a linearity
    // preserving model
    if ((m_pCoordSys != pi_rObj.m_pCoordSys) &&
        (!m_pCoordSys->GetTransfoModelTo(pi_rObj.m_pCoordSys)->PreservesLinearity()))
        {
        Result = false;
        }
    else
        {
        // Check if the two line share the same coordinate system
        if (m_pCoordSys == pi_rObj.m_pCoordSys)
            {
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
            }
        else
            {
            // They do not share the same coordinate system ... create copy
            HGF2DLine   TempLine(pi_rObj);

            // Change coordiante system of copy
            TempLine.ChangeCoordSys(m_pCoordSys);

            // Compare mathematical member attributes
            Result = (HDOUBLE_EQUAL_EPSILON(m_Intercept, TempLine.m_Intercept)) &&
                     (
                         (
                             m_InvertSlope == TempLine.m_InvertSlope &&
                             HDOUBLE_EQUAL_EPSILON(m_Slope, TempLine.m_Slope)
                         ) ||
                         (
                             m_InvertSlope != TempLine.m_InvertSlope &&
                             TempLine.m_Slope != 0.0 &&
                             HDOUBLE_EQUAL_EPSILON(m_Slope, 1.0 / TempLine.m_Slope)
                         )
                     );
            }
        }

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

    @param pi_rObj Constant reference to a HGF2DLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLine::IsEqualTo(const HGF2DLine& pi_rObj, double pi_Epsilon) const
    {
    HINVARIANTS;

    bool   Result;

    // Two lines to be compared must be related by a linearity
    // preserving model
    if ((m_pCoordSys != pi_rObj.m_pCoordSys) &&
        (!m_pCoordSys->GetTransfoModelTo(pi_rObj.m_pCoordSys)->PreservesLinearity()))
        {
        Result = false;
        }
    else
        {
        // Check if the two line share the same coordinate system
        if (m_pCoordSys == pi_rObj.m_pCoordSys)
            {
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

            }
        else
            {
            // They do not share the same coordinate system ... create copy
            HGF2DLine   TempLine(pi_rObj);

            // Change coordiante system of copy
            TempLine.ChangeCoordSys(m_pCoordSys);

            // Compare mathematical member attributes
            Result = (HDOUBLE_EQUAL(m_Intercept, TempLine.m_Intercept, pi_Epsilon)) &&
                     (
                         (
                             m_InvertSlope == TempLine.m_InvertSlope &&
                             HDOUBLE_EQUAL(m_Slope, TempLine.m_Slope, pi_Epsilon)
                         ) ||
                         (
                             m_InvertSlope != TempLine.m_InvertSlope &&
                             TempLine.m_Slope != 0.0 &&
                             HDOUBLE_EQUAL(m_Slope, 1.0 / TempLine.m_Slope, pi_Epsilon)
                         )
                     );
            }
        }

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

    @param pi_rObj Constant reference to a HGF2DLine to compare.

    @return true if lines are identical and false otherwise.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DLine::IsEqualToAutoEpsilon(const HGF2DLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // Two lines to be compared must be related by a linearity
    // preserving model
    if ((m_pCoordSys != pi_rObj.m_pCoordSys) &&
        (!m_pCoordSys->GetTransfoModelTo(pi_rObj.m_pCoordSys)->PreservesLinearity()))
        {
        Result = false;
        }
    else
        {
        // Check if the two line share the same coordinate system
        if (m_pCoordSys == pi_rObj.m_pCoordSys)
            {
            // They do have the same coordinate system ... compare members
            Result = (HNumeric<double>::EQUAL_AUTO_EPSILON(m_Intercept, pi_rObj.m_Intercept)) &&
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
            }
        else
            {
            // They do not share the same coordinate system ... create copy
            HGF2DLine   TempLine(pi_rObj);

            // Change coordiante system of copy
            TempLine.ChangeCoordSys(m_pCoordSys);

            // Compare mathematical member attributes
            Result = (HNumeric<double>::EQUAL_AUTO_EPSILON(m_Intercept, TempLine.m_Intercept)) &&
                     (
                         (
                             m_InvertSlope == TempLine.m_InvertSlope &&
                             HDOUBLE_EQUAL(m_Slope, TempLine.m_Slope, fabs(HGLOBAL_EPSILON * m_Slope))
                         ) ||
                         (
                             m_InvertSlope != TempLine.m_InvertSlope &&
                             TempLine.m_Slope != 0.0 &&
                             HDOUBLE_EQUAL(m_Slope, 1.0 / TempLine.m_Slope, fabs(HGLOBAL_EPSILON * m_Slope))
                         )
                     );
            }
        }

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
bool HGF2DLine::IsParallelTo (const HGF2DLine& pi_rObj) const
    {
    HINVARIANTS;

    bool   Result;

    // Two lines to be compared must be related by a linearity
    // preserving model
    if ((m_pCoordSys != pi_rObj.m_pCoordSys) &&
        (!m_pCoordSys->GetTransfoModelTo(pi_rObj.m_pCoordSys)->PreservesLinearity()))
        {
        Result = false;
        }
    else
        {
        // Check if the two line share the same coordinate system
        if (m_pCoordSys == pi_rObj.m_pCoordSys)
            {
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
            }
        else
            {
            // They do not share the same coordinate system ... create copy
            HGF2DLine   TempLine(pi_rObj);

            // Change coordiante system of copy
            TempLine.ChangeCoordSys(m_pCoordSys);

            // Compare mathematical member attributes
            Result = (
                         (
                             m_InvertSlope == TempLine.m_InvertSlope &&
                             HDOUBLE_EQUAL_EPSILON(m_Slope, TempLine.m_Slope)
                         ) ||
                         (
                             m_InvertSlope != TempLine.m_InvertSlope &&
                             TempLine.m_Slope != 0.0 &&
                             HDOUBLE_EQUAL_EPSILON(m_Slope, 1.0 / TempLine.m_Slope)
                         )
                     );
            }
        }

    return (Result);
    }



/** -----------------------------------------------------------------------------
    This method returns the bearing of the line. Two bearings could be returned
    by the method, and the bearing returned is either one of them.

    @return The bearing of the line
    -----------------------------------------------------------------------------
*/
HGFBearing HGF2DLine::CalculateBearing() const
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
            ReturnBearing = PI / 2;
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
HGF2DLocation HGF2DLine::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // Make sure given point is in same coordinate system
    HGF2DLocation       TempPoint(pi_rPoint, m_pCoordSys);

    HGF2DLocation       ReturnPoint(m_pCoordSys);


#if (0)


    if (m_InvertSlope)
        {
        // The slope is inverted ...

        // is intercept in X and Y of given point
        ReturnPoint.SetX(m_Intercept);
        ReturnPoint.SetY(TempPoint.GetY());
        }
    else
        {
        // The line is not vertical
        // Check if we have inverted slope
        if (m_InvertSlope)
            {
            HGF2DLocation       InterceptPoint(m_Intercept, 0.0, m_pCoordSys);
            HGFBearing          LineBearing(CalculateBearing());
            LineBearing += PI / 2;
            HGF2DDisplacement   PointToIntercept(TempPoint - InterceptPoint);

            HGF2DDisplacement tempDisplacement;
            tempDisplacement.SetByBearing(LineBearing, cos(LineBearing - PointToIntercept.CalculateBearing()) * PointToIntercept.CalculateLength()
            ReturnPoint = InterceptPoint + tempDisplacement;
            }
        else
            {
            HGF2DLocation       InterceptPoint(0.0, m_Intercept, m_pCoordSys);
            HGFBearing          LineBearing(CalculateBearing());
            HGF2DDisplacement   PointToIntercept(TempPoint - InterceptPoint);


            HGF2DDisplacement tempDisplacement;
            tempDisplacement.SetByBearing(LineBearing, cos(LineBearing - PointToIntercept.CalculateBearing()) * PointToIntercept.CalculateLength()
            ReturnPoint = InterceptPoint +tempDisplacement;
            }
        }
#else

    HGF2DLocation APoint(GetCoordSys());

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

    double TempPointX = TempPoint.GetX();
    double TempPointY = TempPoint.GetY();

    double APointX = APoint.GetX();
    double APointY = APoint.GetY();


    double k2 =    ( DeltaX * (TempPointY - APointY) + DeltaY * (APointX - TempPointX) ) * -InvSlopeSqLen;

    double ResultX = TempPointX - k2 * DeltaY;
    double ResultY = TempPointY + k2 * DeltaX;

    ReturnPoint.SetX(ResultX);
    ReturnPoint.SetY(ResultY);

#endif

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
double HGF2DLine::CalculateShortestDistance(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // Make sure given point is in same coordinate system
    HGF2DLocation       TempPoint(pi_rPoint, m_pCoordSys);

    double         ReturnValue;
    // Convert line description to Ax + BY + C
    // formula is mx -y + b
    // A = m
    // B = -1
    // C = b
    // and shortest distance is Ax + By + C / sqrt(A^2 + B^2)
    // which is mx - y + b / sqrt(m^2 + 1)

    if (m_InvertSlope)
        ReturnValue =  fabs((m_Slope * TempPoint.GetY() - TempPoint.GetX() + m_Intercept) / sqrt(m_Slope * m_Slope + 1));
    else
        ReturnValue =  fabs((m_Slope * TempPoint.GetX() - TempPoint.GetY() + m_Intercept) / sqrt(m_Slope * m_Slope + 1));

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

    @param pi_rLine IN Constant reference to the HGF2DLine to find crossing point with.

    @param po_pPoint OUT Pointer to HGF2DLocation that receives the crossing point
                         if there is one.

    @return The status of the crossing operation. This status is
            HGF2DLine::CROSS_FOUND if a crossing point is found,
            HGF2DLine::PARALLEL if the line is parallel to the line given.
            Note that the value of the HGF2DLocation pointed to by po_pPoint is
            undefined if the returned value is different from HGF2DLine::CROSS_FOUND.
    -----------------------------------------------------------------------------
*/
HGF2DLine::CrossState HGF2DLine::IntersectLine(const HGF2DLine& pi_rLine, HGF2DLocation* po_pPoint) const
    {
    HINVARIANTS;

    // Two lines to be compared must be related by a linearity
    // preserving model
    HPRECONDITION((m_pCoordSys == pi_rLine.m_pCoordSys) ||
                  (m_pCoordSys->GetTransfoModelTo(pi_rLine.m_pCoordSys)->PreservesLinearity()));

    HPRECONDITION(po_pPoint != 0);

    // Set the returned point coordinate system
    po_pPoint->SetCoordSys(m_pCoordSys);

    CrossState  TheState;

    // Check if the two line share the same coordinate system
    if (m_pCoordSys == pi_rLine.m_pCoordSys)
        {

        // They do have the same coordinate system ... check that lines are not parallel
#if (0)
        TheState = ((((m_Vertical) && (m_Vertical == pi_rLine.m_Vertical)) ||
                     ((!m_Vertical && !pi_rLine.m_Vertical) && (HDOUBLE_EQUAL(m_Slope, pi_rLine.m_Slope, HGLOBAL_EPSILON / 100.0)))) ?
                    PARALLEL : CROSS_FOUND);
#else
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

#endif

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
        }
    else
        {
        // They do not share the same coordinate system ... create copy
        HGF2DLine   TempLine(pi_rLine);

        // Change coordinate system of copy
        TempLine.ChangeCoordSys(m_pCoordSys);

        // Recursive call with appropriate coordinate systems
        TheState = IntersectLine(TempLine, po_pPoint);
        }

    return (TheState);
    }


/** -----------------------------------------------------------------------------
    This method changes the coordinate system used in the interpretation of the
    line. The definition of the line is modified to maintain the line definition
    in the previous coordinate system used. The transformation model expressing
    the relation between old and new coordinate system must preserve linearity.

    @param pi_rpCoordSys IN Reference to smart pointer to new HGF2DCoordSys to
                            link to line.

    @see GetCoordSys()
    @see SetCoordSys()
    -----------------------------------------------------------------------------
*/
void HGF2DLine::ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    HINVARIANTS;

    // Two lines to be compared must be related by a linearity
    // preserving model
    HPRECONDITION((m_pCoordSys == pi_rpCoordSys) ||
                  (m_pCoordSys->GetTransfoModelTo(pi_rpCoordSys)->PreservesLinearity()));

    // Generate two points
    HGF2DLocation Intercept(m_pCoordSys);
    HGF2DLocation OtherPoint(m_pCoordSys);

    // Intercept point
    if (m_InvertSlope)
        {
        Intercept.SetX(m_Intercept);
        Intercept.SetY(0.0);
        }
    else
        {
        Intercept.SetX(0.0);
        Intercept.SetY(m_Intercept);
        }

    // Arbitrary distance on line point
    HGF2DDisplacement tempDisplacement;
    tempDisplacement.SetByBearing(CalculateBearing(), 1000);
    OtherPoint = (Intercept + tempDisplacement);

    // Change their coordinate system
    Intercept.ChangeCoordSys(pi_rpCoordSys);
    OtherPoint.ChangeCoordSys(pi_rpCoordSys);

    // Set coordinate system
    m_pCoordSys = pi_rpCoordSys;

    // Compute displacement (after transformation)
    HGF2DDisplacement   MyDisplacement(Intercept - OtherPoint);


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
        m_Intercept = Intercept.GetY() - (Intercept.GetX() * m_Slope);

        }
    else
        {
        // Line has a high slope ... we will invert parameters
        m_InvertSlope = true;

        // Compute slope
        m_Slope = (MyDisplacement.GetDeltaX() / MyDisplacement.GetDeltaY());

        // Compute intercept
        m_Intercept = Intercept.GetX() - (Intercept.GetY() * m_Slope);
        }

    }


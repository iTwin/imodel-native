//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DCircle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Methods for class HVE2DCircle
//----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DCircle.h>

HPM_REGISTER_CLASS(HVE2DCircle, HVE2DSimpleShape)


#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HVE2DArc.h>


//-----------------------------------------------------------------------------
// operator==
// Equality compare operator.
//-----------------------------------------------------------------------------
bool HVE2DCircle::operator==(const HVE2DCircle& pi_rObj) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rObj.GetCoordSys()) ||
                  (GetCoordSys()->GetTransfoModelTo(pi_rObj.GetCoordSys())->PreservesShape()));

    // Check if centers are located at same position
    bool   Answer = (m_Center == pi_rObj.m_Center);

    // If centers are equal ...
    if (Answer)
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rObj.GetCoordSys())
            {
            // They have the same coordinate system ...
            Answer = (m_Radius == pi_rObj.m_Radius);
            }
        else
            {
            HVE2DCircle TempCircle(pi_rObj);
            TempCircle.ChangeCoordSys(GetCoordSys());

            Answer = operator==(TempCircle);
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HVE2DCircle::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    HINVARIANTS;
    // Set ancester coordsys
    HVE2DSimpleShape::SetCoordSysImplementation(pi_rpCoordSys);

    // Set center coord sys
    m_Center.SetCoordSys(GetCoordSys());
    }


//-----------------------------------------------------------------------------
// IsEqualTo
// Equality compare operator with epsilon application.
//-----------------------------------------------------------------------------
bool HVE2DCircle::IsEqualTo(const HVE2DCircle& pi_rObj) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rObj.GetCoordSys()) ||
                  (GetCoordSys()->GetTransfoModelTo(pi_rObj.GetCoordSys())->PreservesShape()));

    // Check if centers are located at same position
    bool   Answer = m_Center.IsEqualTo(pi_rObj.m_Center);

    // If centers are equal ...
    if (Answer)
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rObj.GetCoordSys())
            {
            // They have the same coordinate system ...
            // Compare radius
            Answer = m_Radius.IsEqualTo(pi_rObj.m_Radius);
            }
        else
            {
            HVE2DCircle TempCircle(pi_rObj);
            TempCircle.ChangeCoordSys(GetCoordSys());

            Answer = IsEqualTo(TempCircle);
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsEqualTo
// Equality compare operator with epsilon application.
//-----------------------------------------------------------------------------
bool HVE2DCircle::IsEqualTo(const HVE2DCircle& pi_rObj,
                             double            pi_Epsilon) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rObj.GetCoordSys()) ||
                  (GetCoordSys()->GetTransfoModelTo(pi_rObj.GetCoordSys())->PreservesShape()));

    // Check if centers are located at same position
    bool   Answer = m_Center.IsEqualTo(pi_rObj.m_Center, pi_Epsilon);

    // If centers are equal ...
    if (Answer)
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rObj.GetCoordSys())
            {
            // They have the same coordinate system ...
            // Compare radius
            Answer = m_Radius.IsEqualTo(pi_rObj.m_Radius, pi_Epsilon);
            }
        else
            {
            HVE2DCircle TempCircle(pi_rObj);
            TempCircle.ChangeCoordSys(GetCoordSys());

            Answer = IsEqualTo(TempCircle, pi_Epsilon);
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsEqualToAutoEpsilon
// Equality compare operator with epsilon application.
//-----------------------------------------------------------------------------
bool HVE2DCircle::IsEqualToAutoEpsilon(const HVE2DCircle& pi_rObj) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // They must either have the same coordinate system or be related
    // through a shape preserving relation.
    HPRECONDITION((GetCoordSys() == pi_rObj.GetCoordSys()) ||
                  (GetCoordSys()->GetTransfoModelTo(pi_rObj.GetCoordSys())->PreservesShape()));

    // Check if centers are located at same position
    bool   Answer = m_Center.IsEqualToAutoEpsilon(pi_rObj.m_Center);

    // If centers are equal ...
    if (Answer)
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rObj.GetCoordSys())
            {
            // They have the same coordinate system ...
            // Compare radius
            Answer = m_Radius.IsEqualToAutoEpsilon(pi_rObj.m_Radius);
            }
        else
            {
            HVE2DCircle TempCircle(pi_rObj);
            TempCircle.ChangeCoordSys(GetCoordSys());

            Answer = IsEqualToAutoEpsilon(TempCircle);
            }
        }

    return(Answer);
    }



//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DCircle::GetLinear() const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    // Cut the circle in two at arbitrary direction
    HGFBearing   FirstBearing(0.0);
    HGFBearing   SecondBearing(PI);

    // Add required parts
    MyComplexLinear.AppendLinear(HVE2DArc(m_Center, FirstBearing, PI, m_Radius));
    MyComplexLinear.AppendLinear(HVE2DArc(m_Center, SecondBearing, PI, m_Radius));

    return(MyComplexLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DCircle::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    // Set angle direction desired
    double AngleValue = (pi_DirectionDesired == HVE2DSimpleShape::CW ? -PI : PI);


    // Cut the circle in two at arbitrary direction
    HGFBearing   FirstBearing(0.0);
    HGFBearing   SecondBearing(PI);

    // Add required parts
    MyComplexLinear.AppendLinear(HVE2DArc(m_Center, FirstBearing, AngleValue, m_Radius));
    MyComplexLinear.AppendLinear(HVE2DArc(m_Center, SecondBearing, AngleValue, m_Radius));


    return(MyComplexLinear);
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear* HVE2DCircle::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // Allocate linear
    HVE2DComplexLinear*    pMyComplexLinear = new HVE2DComplexLinear(GetCoordSys());

    // Set angle direction desired
    double AngleValue = (pi_DirectionDesired == HVE2DSimpleShape::CW ? -PI : PI);


    // Cut the circle in two at arbitrary direction
    HGFBearing   FirstBearing(0.0);
    HGFBearing   SecondBearing(PI);

    // Add required parts
    pMyComplexLinear->AppendLinear(HVE2DArc(m_Center, FirstBearing, AngleValue, m_Radius));
    pMyComplexLinear->AppendLinear(HVE2DArc(m_Center, SecondBearing, AngleValue, m_Radius));


    return(pMyComplexLinear);
    }



//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DCircle::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HINVARIANTS;

    HVE2DVector*    pMyResultVector;

    // Obtain the transformation model to the given coordinate system
    HFCPtr<HGF2DTransfoModel> pMyModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);

    // Check if this model preserves shape
    if (pMyModel->PreservesShape())
        {
        // Calculate transformed perimeter point (arbitrary point)
        HGF2DLocation   ModifiedCenter(m_Center, pi_rpCoordSys);
        HGF2DLocation   OtherPoint = m_Center +
                                     HGF2DDisplacement(HGFBearing(0.0),
                                                       m_Radius);

        double   ModifiedRadius = (ModifiedCenter - OtherPoint).CalculateLength();

        // The model preserves shape ... we can transform the points directly
        pMyResultVector = new HVE2DCircle(ModifiedCenter, ModifiedRadius);
        }
    else
        {
        // The model either does not preserve the shape
        // We transform the circle into a polygon, and pass and tell the polygon to
        // process itself
        HVE2DPolygon    ThePolygon(GetLinear());
        pMyResultVector = ThePolygon.AllocateCopyInCoordSys(pi_rpCoordSys);
        }

    return(pMyResultVector);
    }





//-----------------------------------------------------------------------------
// SetByPerimeterPoints
//-----------------------------------------------------------------------------
void HVE2DCircle::SetByPerimeterPoints(const HGF2DLocation& pi_rFirstPoint,
                                       const HGF2DLocation& pi_rSecondPoint,
                                       const HGF2DLocation& pi_rThirdPoint)
    {
    HINVARIANTS;
    // The three points must be different
    HPRECONDITION(!pi_rFirstPoint.IsEqualTo(pi_rSecondPoint));
    HPRECONDITION(!pi_rFirstPoint.IsEqualTo(pi_rThirdPoint));
    HPRECONDITION(!pi_rSecondPoint.IsEqualTo(pi_rThirdPoint));

    // The three points must not be co-linear
    HPRECONDITION((pi_rFirstPoint - pi_rSecondPoint).CalculateBearing() !=
                  (pi_rFirstPoint - pi_rThirdPoint).CalculateBearing());

    // Obtain coordinate of center of segment formed from point 1 to point 2
    HGF2DDisplacement FromFirstToSecond = (pi_rSecondPoint - pi_rFirstPoint);
    HGF2DLocation TempPoint1 = pi_rFirstPoint + (FromFirstToSecond / 2);

    // Calculate bearing
    HGFBearing FirstToSecondBearing = FromFirstToSecond.CalculateBearing();

    // Create line from point and bearing
    HGF2DLine  FirstLine(TempPoint1, FirstToSecondBearing + PI / 2);

    // Obtain coordinate of center of segment formed from point 2 to point 3
    HGF2DDisplacement FromSecondToThird = (pi_rThirdPoint - pi_rSecondPoint);
    TempPoint1 = pi_rSecondPoint + (FromSecondToThird / 2);

    // Calculate bearing
    HGFBearing SecondToThirdBearing = FromSecondToThird.CalculateBearing();

    // Create line from point and bearing
    HGF2DLine  SecondLine(TempPoint1, SecondToThirdBearing + PI / 2);

    // Obtain intersection point
    HGF2DLine::CrossState MyState = FirstLine.IntersectLine(SecondLine, &m_Center);

    HASSERT(MyState != HGF2DLine::PARALLEL);

    // Obtain radius
    m_Radius = (m_Center - pi_rFirstPoint).CalculateLength();

    ResetTolerance();

    // Make sure the radius is equal for all distance to given points
    HPOSTCONDITION(HDOUBLE_EQUAL(m_Radius, (m_Center - pi_rSecondPoint).CalculateLength(), GetTolerance()));
    HPOSTCONDITION(HDOUBLE_EQUAL(m_Radius, (m_Center - pi_rThirdPoint).CalculateLength(), GetTolerance()));
    }


//-----------------------------------------------------------------------------
// IntersectLine
//-----------------------------------------------------------------------------
uint32_t HVE2DCircle::IntersectLine(const HGF2DLine& pi_rLine,
                                   HGF2DLocation* po_pFirstCrossPoint,
                                   HGF2DLocation* po_pSecondCrossPoint) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(po_pFirstCrossPoint != NULL);
    HPRECONDITION(po_pSecondCrossPoint != NULL);

    // If coordinate systems are different, they must linear preserving related
    HPRECONDITION((GetCoordSys() == pi_rLine.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rLine.GetCoordSys())->PreservesLinearity());

    int32_t NumberOfCrossPoints = 0;

    // Make a copy of given line
    HGF2DLine WorkLine(pi_rLine);

    if (GetCoordSys() != pi_rLine.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkLine.ChangeCoordSys(GetCoordSys());
        }

#if (0)
    // Obtain raw coordinates of center
    double x0 = m_Center.GetX();
    double y0 = m_Center.GetY();
    double WorkRadius = m_Radius;

    // Check if line is vertical
    if (WorkLine.IsVertical())
        {
        // Line is vertical ...
        // Obtain intercept of line
        double c = WorkLine.GetIntercept();

        double Determ = (WorkRadius * WorkRadius) - ((c - x0) * (c - x0));

        // Check that determinant is greater than 0
        if (Determ > 0.0)
            {
            // There are two cross point
            NumberOfCrossPoints = 2;

            // Set coordinate system of returned point
            po_pFirstCrossPoint->SetCoordSys(GetCoordSys());
            po_pSecondCrossPoint->SetCoordSys(GetCoordSys());

            // Find the square root of determinant
            double SqrtDeterm = sqrt(Determ);

            // Calculate first cross point
            po_pFirstCrossPoint->SetX(c);
            po_pFirstCrossPoint->SetY(SqrtDeterm + y0);

            // Calculate second cross point
            po_pSecondCrossPoint->SetX(c);
            po_pSecondCrossPoint->SetY(-SqrtDeterm + y0);

            HASSERT(IsPointOn(*po_pFirstCrossPoint));
            HASSERT(IsPointOn(*po_pSecondCrossPoint));
            }
        }
    else
        {
        // First we obtain the slope of line
        double a = WorkLine.GetSlope();

        // Obtain methematical parameters (in X unit value)

        // Obtain intercept
        double b = WorkLine.GetIntercept();

        // Compute temporary math params
        double A = (a * a + 1.0);
        double B = (-2 * x0 + 2 * a * b - 2 * a * y0);
        double C = (x0 * x0 - 2 * b * y0 + b * b + y0 * y0 - WorkRadius * WorkRadius);

        HASSERT(A != 0.0);

        // Calculate determinant
        double Determ = B * B - 4 * A * C;

        // If determinant is negative or zero ... no cross
        if (Determ > 0.0)
            {
            // There are 2 cross points
            NumberOfCrossPoints = 2;

            // Set coordinate system of returned point
            po_pFirstCrossPoint->SetCoordSys(GetCoordSys());
            po_pSecondCrossPoint->SetCoordSys(GetCoordSys());

            // Find the square root of determinant
            double SqrtDeterm = sqrt(Determ);

            // Calculate first cross point
            po_pFirstCrossPoint->SetX((-B + SqrtDeterm) / (2 * A));
            po_pFirstCrossPoint->SetY(a * po_pFirstCrossPoint->GetX() + WorkLine.GetIntercept());

            // Calculate second cross point
            po_pSecondCrossPoint->SetX((-B - SqrtDeterm) / (2 * A));
            po_pSecondCrossPoint->SetY(a * po_pSecondCrossPoint->GetX() + WorkLine.GetIntercept());

            HASSERT(IsPointOn(*po_pFirstCrossPoint));
            HASSERT(IsPointOn(*po_pSecondCrossPoint));

            }
        }
#else
    // Obtain raw coordinates of center
    double X0 = m_Center.GetX();
    double Y0 = m_Center.GetY();
    double WorkRadius = m_Radius;

    double X1;
    double Y1;
    double X2;
    double Y2;

    // Check if line is vertical
    if (WorkLine.IsVertical())
        {
        // Line is vertical ...
        // Obtain intercept of line
        double Intercept = WorkLine.GetIntercept();

        X1 = Intercept;
        Y1 = 0.0;
        X2 = Intercept;
        Y2 = 1000.0;
        }
    else
        {
        // First we obtain the slope of line
        double Slope = WorkLine.GetSlope();
        double Intercept = WorkLine;

        // Create arbitrary points
        X1 = 0.0;
        Y1 = Intercept;
        X2 = 1000.0;
        Y2 = 1000.0 * Slope + Intercept;
        }

    // According to Bourke
    double a = ((X2 - X1) * (X2 - X1)) + ((Y2 - Y1) * (Y2 - Y1));
    double b = 2 * ((X2 - X1)*(X1 - X0) + (Y2 - Y1)*(Y1 - Y0));
    double c = X0*X0 + Y0*Y0 + X1*X1 + Y1*Y1 -2*(X0*X1 + Y0*Y1)-(WorkRadius * WorkRadius);

    double Determ = b*b - 4*a*c;

    // Intersection if determinant is greater than 0.0
    if (Determ > 0.0)
        {
        double Root = sqrt(Determ);

        double u1 = (-b + Root) / (2*a);
        double u2 = (-b - Root) / (2*a);

        HGF2DLocation P1(X1, Y1, GetCoordSys());
        HGF2DLocation P2(X2, Y2, GetCoordSys());

        HGF2DDisplacement MyDisp = P2 - P1;
        *po_pFirstCrossPoint = P1 + (u1 * MyDisp);
        *po_pSecondCrossPoint = P1 + (u2 * MyDisp);

        HASSERT(IsPointOn(*po_pFirstCrossPoint));
        HASSERT(IsPointOn(*po_pSecondCrossPoint));

        NumberOfCrossPoints = 2;
        }
#endif

    return (NumberOfCrossPoints);
    }


//-----------------------------------------------------------------------------
// IntersectSegment
//-----------------------------------------------------------------------------
uint32_t HVE2DCircle::IntersectSegment(const HVE2DSegment& pi_rSegment,
                                      HGF2DLocation* po_pFirstCrossPoint,
                                      HGF2DLocation* po_pSecondCrossPoint) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(po_pFirstCrossPoint != NULL);
    HPRECONDITION(po_pSecondCrossPoint != NULL);

    // If coordinate systems are different, they must linear preserving related
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rSegment.GetCoordSys())->PreservesLinearity());

    int32_t NumberOfCrossPoints;

    // Obtain intersection with line of segment
    int32_t TempNumberOfCrossPoints = IntersectLine(HGF2DLine(pi_rSegment.GetStartPoint(),
                                                              pi_rSegment.GetEndPoint()),
                                                    po_pFirstCrossPoint,
                                                    po_pSecondCrossPoint);

    int32_t OrigNumberOfCrossPoints = TempNumberOfCrossPoints;

    // Check if there was any cross point
    if (TempNumberOfCrossPoints > 0)
        {
        HASSERT(TempNumberOfCrossPoints == 2);

        // Check if first point is on segment
        if (!pi_rSegment.IsPointOn(*po_pFirstCrossPoint) ||
            pi_rSegment.GetStartPoint().IsEqualTo(*po_pFirstCrossPoint) ||
            pi_rSegment.GetEndPoint().IsEqualTo(*po_pFirstCrossPoint))
            {
            // The cross point is invalid ... we remove
            TempNumberOfCrossPoints--;
            *po_pFirstCrossPoint = *po_pSecondCrossPoint;
            }

        // Check if second point is on segment
        if (!pi_rSegment.IsPointOn(*po_pSecondCrossPoint) ||
            pi_rSegment.GetStartPoint().IsEqualTo(*po_pSecondCrossPoint) ||
            pi_rSegment.GetEndPoint().IsEqualTo(*po_pSecondCrossPoint))
            {
            // The second point is invalid
            TempNumberOfCrossPoints--;
            }

        // Set final number of cross points
        NumberOfCrossPoints = TempNumberOfCrossPoints;
        }

    return(NumberOfCrossPoints);

    }


//-----------------------------------------------------------------------------
// IntersectCircle
//-----------------------------------------------------------------------------
uint32_t HVE2DCircle::IntersectCircle(const HVE2DCircle& pi_rCircle,
                                     HGF2DLocation* po_pFirstCrossPoint,
                                     HGF2DLocation* po_pSecondCrossPoint) const
    {
    HINVARIANTS;
    // Neither circles must be empty
    HPRECONDITION(!IsEmpty());
    HPRECONDITION(!pi_rCircle.IsEmpty());

    // Recipient variables must be provided
    HPRECONDITION(po_pFirstCrossPoint != NULL);
    HPRECONDITION(po_pSecondCrossPoint != NULL);

    // If coordinate systems are different, they must shape preserving related
    HPRECONDITION((GetCoordSys() == pi_rCircle.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rCircle.GetCoordSys())->PreservesShape());


    int32_t NumberOfCrossPoints = 0;

    // Make a copy of given circle
    HVE2DCircle WorkCircle(pi_rCircle);

    if (GetCoordSys() != pi_rCircle.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkCircle.ChangeCoordSys(GetCoordSys());
        }

#if (0)

    // First obtain circle parameters (in X unit)
    double Ra = m_Radius;
    double X0a = m_Center.GetX();
    double Y0a = m_Center.GetY();

    double Rb = WorkCircle.m_Radius;
    double X0b = WorkCircle.m_Center.GetX();
    double Y0b = WorkCircle.m_Center.GetY();


    // Compute mathematical parameters
    double X0Diff = (X0b - X0a);
    double X0Diff2 = X0Diff * X0Diff;
    double Y0a2 = Y0a * Y0a;
    double Y0b2 = Y0b * Y0b;
    double Ra2 = Ra * Ra;
    double Rb2 = Rb * Rb;
    double A = (-4 * X0Diff2 - 4 * Y0b2 + 8 * Y0b * Y0a - 4 * Y0a2);
    double B = 4 * (-Y0b * Rb2 +
                     Y0a * Rb2 +
                     Y0b2 * Y0b -
                     Y0b * X0Diff2 -
                     Y0b * Y0a2 +
                     Y0b * Ra2 -
                     Y0a * Y0b2 +
                     Y0a * X0Diff2 +
                     Y0a2 * Y0a -
                     Y0a * Ra2);
    double C = -Rb2 * Rb2 +
                2 * Rb2 * Y0b2 +
                2 * Rb2 * X0Diff2 -
                2 * Rb2 * Y0a2 +
                2 * Rb2 * Ra2 -
                Y0b2 * Y0b2 -
                2 * Y0b2 * X0Diff2 +
                2 * Y0b2 * Y0a2 -
                2 * Y0b2 * Ra2 -
                X0Diff2 * X0Diff2 -
                2 * Y0a2 * X0Diff2 -
                2 * Ra2 * X0Diff2 -
                Y0a2 * Y0a2 +
                2 * Y0a2 * Ra2 -
                Ra2 * Ra2;

    HASSERT(A != 0.0);

    // Calculate determinant
    double Determ = B * B - 4 * A * C;

    // If determinant is negative or zero ... no cross
    if (Determ > 0.0)
        {
        // There are 2 cross points
        NumberOfCrossPoints = 2;

        // Set coordinate system of returned point
        po_pFirstCrossPoint->SetCoordSys(GetCoordSys());
        po_pSecondCrossPoint->SetCoordSys(GetCoordSys());

        // Find the square root of determinant
        double SqrtDeterm = sqrt(Determ);

        // Calculate first cross point
        double Yc = (-B + SqrtDeterm) / 2 * A;
        po_pFirstCrossPoint->SetY(Yc);

        // There are two possible solution for X
        // Try the first solution
        po_pFirstCrossPoint->SetX(sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

        HASSERT(IsPointOn(*po_pFirstCrossPoint));

        // Check if point is on both circles
        if (!WorkCircle.IsPointOn(*po_pFirstCrossPoint))
            {
            // This point is not on ... try other solution
            po_pFirstCrossPoint->SetX(-sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

            HASSERT(IsPointOn(*po_pFirstCrossPoint));
            HASSERT(WorkCircle.IsPointOn(*po_pFirstCrossPoint));
            }

        // Calculate second cross point
        Yc = (-B - SqrtDeterm) / (2 * A);
        po_pSecondCrossPoint->SetY(Yc);

        // There are two possible solution for X
        // Try the first solution
        po_pSecondCrossPoint->SetX(sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

        HASSERT(IsPointOn(*po_pSecondCrossPoint));

        // Check if point is on both circles
        if (!WorkCircle.IsPointOn(*po_pSecondCrossPoint))
            {
            // This point is not on ... try other solution
            po_pSecondCrossPoint->SetX(-sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

            HASSERT(IsPointOn(*po_pSecondCrossPoint));
            HASSERT(WorkCircle.IsPointOn(*po_pSecondCrossPoint));
            }

        // Verify that the two points are different
        HASSERT(*po_pFirstCrossPoint != *po_pSecondCrossPoint);
        }
#else

    // Check if their center is equal ... this increases precision
    if (!m_Center.IsEqualTo(WorkCircle.GetCenter()))
        {
        // According to Bourke ...

        // Compute distances from two centers
        double D = (m_Center - WorkCircle.GetCenter()).CalculateLength();

        // Check that the distances between centers is smaller than sum of radius
        if (D < (m_Radius + WorkCircle.GetRadius()))
            {
            // Modification to Bourke ...
            // If D is smaller that difference of radius then one
            // circle is completely included in the other and thus cannot intersect
            if (D > fabs(m_Radius - WorkCircle.GetRadius()))
                {
                // Crossings

                double A = (m_Radius * m_Radius - (WorkCircle.GetRadius() * WorkCircle.GetRadius()) + D * D) / (2 * D);

                // Intermediate point
                HGF2DLocation P2 = m_Center + (A / D) * (WorkCircle.GetCenter() - m_Center);

                // Compute Height from P2
                double H = sqrt(m_Radius * m_Radius - A * A);

                // Calculate coordinates
                double X3_1 = P2.GetX() + (H / D) * (WorkCircle.GetCenter().GetY() - m_Center.GetY());
                double X3_2 = P2.GetX() - (H / D) * (WorkCircle.GetCenter().GetY() - m_Center.GetY());

                double Y3_1 = P2.GetY() - (H / D) * (WorkCircle.GetCenter().GetX() - m_Center.GetX());
                double Y3_2 = P2.GetY() + (H / D) * (WorkCircle.GetCenter().GetX() - m_Center.GetX());

                // There are 2 cross points
                NumberOfCrossPoints = 2;
                *po_pFirstCrossPoint = HGF2DLocation(X3_1, Y3_1, GetCoordSys());
                *po_pSecondCrossPoint = HGF2DLocation(X3_1, Y3_1, GetCoordSys());
                }
            }
        }
#endif

    return(NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// IsAdjacentToCircle
// This method indicates if the two circles are adjacent one to the other
//-----------------------------------------------------------------------------
bool  HVE2DCircle::IsAdjacentToCircle(const HVE2DCircle& pi_rCircle) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // If coordinate systems are different, they must shape preserving related
    HPRECONDITION((GetCoordSys() == pi_rCircle.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rCircle.GetCoordSys())->PreservesShape());

    // Make a copy of given circle
    HVE2DCircle WorkCircle(pi_rCircle);

    if (GetCoordSys() != pi_rCircle.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkCircle.ChangeCoordSys(GetCoordSys());
        }

    // First obtain circle parameters (in X unit)
    double Ra = m_Radius;
    double X0a = m_Center.GetX();
    double Y0a = m_Center.GetY();

    double Rb = WorkCircle.m_Radius;
    double X0b = WorkCircle.m_Center.GetX();
    double Y0b = WorkCircle.m_Center.GetY();


    // Compute mathematical parameters
    double X0Diff = (X0b - X0a);
    double X0Diff2 = X0Diff * X0Diff;
    double Y0a2 = Y0a * Y0a;
    double Y0b2 = Y0b * Y0b;
    double Ra2 = Ra * Ra;
    double Rb2 = Rb * Rb;
    double A = (-4 * X0Diff2 - 4 * Y0b2 + 8 * Y0b * Y0a - 4 * Y0a2);
    double B = 4 * (-Y0b * Rb2 +
                     Y0a * Rb2 +
                     Y0b2 * Y0b -
                     Y0b * X0Diff2 -
                     Y0b * Y0a2 +
                     Y0b * Ra2 -
                     Y0a * Y0b2 +
                     Y0a * X0Diff2 +
                     Y0a2 * Y0a -
                     Y0a * Ra2);
    double C = -Rb2 * Rb2 +
                2 * Rb2 * Y0b2 +
                2 * Rb2 * X0Diff2 -
                2 * Rb2 * Y0a2 +
                2 * Rb2 * Ra2 -
                Y0b2 * Y0b2 -
                2 * Y0b2 * X0Diff2 +
                2 * Y0b2 * Y0a2 -
                2 * Y0b2 * Ra2 -
                X0Diff2 * X0Diff2 -
                2 * Y0a2 * X0Diff2 -
                2 * Ra2 * X0Diff2 -
                Y0a2 * Y0a2 +
                2 * Y0a2 * Ra2 -
                Ra2 * Ra2;

    HASSERT(A != 0.0);

    // Calculate determinant
    double Determ = B * B - 4 * A * C;

    // If determinant is zero ... they are adjacent
    return(Determ == 0.0);
    }

//-----------------------------------------------------------------------------
// IsAdjacentToLine
// This method indicates if the circle and line are adjacent one to the other
//-----------------------------------------------------------------------------
bool  HVE2DCircle::IsAdjacentToLine(const HGF2DLine& pi_rLine) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    // If coordinate systems are different, they must shape preserving related
    HPRECONDITION((GetCoordSys() == pi_rLine.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rLine.GetCoordSys())->PreservesLinearity());

    // Make a copy of given line
    HGF2DLine WorkLine(pi_rLine);

    if (GetCoordSys() != pi_rLine.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkLine.ChangeCoordSys(GetCoordSys());
        }

    double Determ;

    // Obtain mathematical parameters (in X unit value)
    // Obtain raw coordinates of center
    double x0 = m_Center.GetX();
    double y0 = m_Center.GetY();

    double WorkRadius = m_Radius;

    // Check if line is vertical
    if (WorkLine.IsVertical())
        {
        // Line is vertical ...
        // Obtain intercept of line
        double c = WorkLine.GetIntercept();

        Determ = (WorkRadius * WorkRadius) - ((c - x0) * (c - x0));
        }
    else
        {
        // First we obtain the slope of line
        double a = WorkLine.GetSlope();


        // Obtain intercept
        double b = WorkLine.GetIntercept();


        // Compute temporary math params
        double A = (a * a + 1.0);
        double B = (-2 * x0 + 2 * a * b - 2 * a * y0);
        double C = (x0 * x0 - 2 * b * y0 + b * b + y0 * y0 - WorkRadius * WorkRadius);

        HASSERT(A != 0.0);

        // Calculate determinant
        Determ = B * B - 4 * A * C;
        }

    // If determinant is equal to 0.0 ... they are adjacent
    return(Determ == 0.0);
    }


//-----------------------------------------------------------------------------
// ObtainCircleAdjacencyPoint
// This method returns the adjacency point of two adjacent
// circles. Note that if the two circles are equal(contiguous), then
// an arbitrary point is returned
//-----------------------------------------------------------------------------
void  HVE2DCircle::ObtainCircleAdjacencyPoint(const HVE2DCircle& pi_rCircle,
                                              HGF2DLocation* po_pAdjacencePoint) const
    {
    HINVARIANTS;
    // Neither circle must be empty
    HPRECONDITION(!IsEmpty());
    HPRECONDITION(!pi_rCircle.IsEmpty());

    HPRECONDITION(po_pAdjacencePoint != NULL);

    // The two circles must be adjacent
    HPRECONDITION(IsAdjacentToCircle(pi_rCircle));

    // If coordinate systems are different, they must shape preserving related
    HPRECONDITION((GetCoordSys() == pi_rCircle.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rCircle.GetCoordSys())->PreservesShape());

    // Make a copy of given circle
    HVE2DCircle WorkCircle(pi_rCircle);

    if (GetCoordSys() != pi_rCircle.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkCircle.ChangeCoordSys(GetCoordSys());
        }

    // First obtain circle parameters (in X unit)
    double Ra = m_Radius;
    double X0a = m_Center.GetX();
    double Y0a = m_Center.GetY();

    double Rb = WorkCircle.m_Radius;
    double X0b = WorkCircle.m_Center.GetX();
    double Y0b = WorkCircle.m_Center.GetY();


    // Compute mathematical parameters
    double X0Diff = (X0b - X0a);
    double X0Diff2 = X0Diff * X0Diff;
    double Y0a2 = Y0a * Y0a;
    double Y0b2 = Y0b * Y0b;
    double Ra2 = Ra * Ra;
    double Rb2 = Rb * Rb;
    double A = (-4 * X0Diff2 - 4 * Y0b2 + 8 * Y0b * Y0a - 4 * Y0a2);
    double B = 4 * (-Y0b * Rb2 +
                     Y0a * Rb2 +
                     Y0b2 * Y0b -
                     Y0b * X0Diff2 -
                     Y0b * Y0a2 +
                     Y0b * Ra2 -
                     Y0a * Y0b2 +
                     Y0a * X0Diff2 +
                     Y0a2 * Y0a -
                     Y0a * Ra2);
    double C = -Rb2 * Rb2 +
                2 * Rb2 * Y0b2 +
                2 * Rb2 * X0Diff2 -
                2 * Rb2 * Y0a2 +
                2 * Rb2 * Ra2 -
                Y0b2 * Y0b2 -
                2 * Y0b2 * X0Diff2 +
                2 * Y0b2 * Y0a2 -
                2 * Y0b2 * Ra2 -
                X0Diff2 * X0Diff2 -
                2 * Y0a2 * X0Diff2 -
                2 * Ra2 * X0Diff2 -
                Y0a2 * Y0a2 +
                2 * Y0a2 * Ra2 -
                Ra2 * Ra2;

    HASSERT(A != 0.0);


    double Yc = -B / (2 * A);
    po_pAdjacencePoint->SetY(Yc);

    // There are two possible solution for X
    // Try the first solution
    po_pAdjacencePoint->SetX(sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

    HASSERT(IsPointOn(*po_pAdjacencePoint));

    // Check if point is on both circles
    if (!WorkCircle.IsPointOn(*po_pAdjacencePoint))
        {
        // This point is not on ... try other solution
        po_pAdjacencePoint->SetX(-sqrt(Ra2 - pow(Yc - Y0a, 2)) + X0a);

        HASSERT(IsPointOn(*po_pAdjacencePoint));
        HASSERT(WorkCircle.IsPointOn(*po_pAdjacencePoint));
        }

    }

//-----------------------------------------------------------------------------
// ObtainLineAdjacencyPoint
// This method returns the adjacency point of two adjacent objects
//-----------------------------------------------------------------------------
void  HVE2DCircle::ObtainLineAdjacencyPoint(const HGF2DLine& pi_rLine,
                                            HGF2DLocation* po_pAdjacencePoint) const
    {
    HINVARIANTS;
    // The circle must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(po_pAdjacencePoint != NULL);

    // The two circles must be adjacent
    HPRECONDITION(IsAdjacentToLine(pi_rLine));

    // If coordinate systems are different, they must shape preserving related
    HPRECONDITION((GetCoordSys() == pi_rLine.GetCoordSys()) ||
                  GetCoordSys()->GetTransfoModelTo(pi_rLine.GetCoordSys())->PreservesLinearity());

    // Set coordinate system of returned point
    po_pAdjacencePoint->SetCoordSys(GetCoordSys());

    // Make a copy of given line
    HGF2DLine WorkLine(pi_rLine);

    // Check if line is in same coordinate system as circle
    if (GetCoordSys() != pi_rLine.GetCoordSys())
        {
        // They are not in the same coordinate system
        WorkLine.ChangeCoordSys(GetCoordSys());
        }

    // Check if line is vertical
    if (WorkLine.IsVertical())
        {
        // Line is vertical
        po_pAdjacencePoint->SetX(WorkLine.GetIntercept());
        po_pAdjacencePoint->SetY(m_Center.GetY());
        }
    else
        {
        // First we obtain the slope of line
        double a = WorkLine.GetSlope();

        // Obtain mathematical parameters (in X unit value)

        // Obtain intercept
        double b = WorkLine.GetIntercept();

        // Obtain raw coordinates of center
        double x0 = m_Center.GetX();
        double y0 = m_Center.GetY();

        double WorkRadius = m_Radius;

        // Compute temporary math params
        double A = (a * a + 1.0);
        double B = (-2 * x0 + 2 * a * b - 2 * a * y0);

        HASSERT(A != 0.0);

        // Adjacent location is at :
        double Xc = -B / (2 * A);

        // We obtain y by using the line equation
        double Yc = a * Xc + b;

        po_pAdjacencePoint->SetX(Xc);
        po_pAdjacencePoint->SetY(Yc);
        }

    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE
// This method recalculates the tolerance of circle if automatic tolerance is
// active
//-----------------------------------------------------------------------------
void HVE2DCircle::ResetTolerance()
    {
    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        // Autotolerance active ... update tolerance

        // Set tolerance to minimum value accepted
        double Tolerance = HGLOBAL_EPSILON;

        // Check if a greater tolerance may be applicable
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetX() - m_Radius));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetX() + m_Radius));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetY() - m_Radius));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetY() + m_Radius));

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DCircle::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char     DumString[256];

    HVE2DSimpleShape::PrintState(po_rOutput);
    HDUMP0("Object is a HVE2Circle\n");
    po_rOutput << "Object is a HVE2Circle" << endl;

    po_rOutput << "Center is : " << endl;
    HDUMP0("Center is : \n");
    sprintf(DumString, "X = %5.15lf  Y = %5.15lf", m_Center.GetX(), m_Center.GetY());
    HDUMP1("Center is at : %s\n", DumString);
    po_rOutput << "Center is at : " << DumString << endl;
    HDUMP1("Radius is : %5.15lf\n", m_Radius);
    po_rOutput << "Radius is : " << m_Radius << endl;


    HDUMP0("END OF COMPONENT LISTING\n");
    po_rOutput << "END OF COMPONENT LISTING" << endl;

#endif
    }

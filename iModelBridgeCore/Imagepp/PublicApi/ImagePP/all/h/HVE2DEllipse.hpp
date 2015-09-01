//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DEllipse.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DEllipse::HVE2DEllipse()
    : m_F1(0.0, 0.0, GetCoordSys()),
      m_F2(0.0, 0.0, GetCoordSys()),
      m_SemiMajorAxis(0.0)
    {
    }

//-----------------------------------------------------------------------------
// Constructor (Circumscript rectangle)
//-----------------------------------------------------------------------------
inline HVE2DEllipse::HVE2DEllipse(const HVE2DRectangle& pi_rRectangle)
    : HVE2DSimpleShape(pi_rRectangle.GetCoordSys())
    {
    double MinX;
    double MinY;
    double MaxX;
    double MaxY;

    pi_rRectangle.GetRectangle(&MinX, &MinY, &MaxX, &MaxY);

    m_Center.SetCoordSys(GetCoordSys());
    m_F1.SetCoordSys(GetCoordSys());
    m_F2.SetCoordSys(GetCoordSys());

    m_SemiMajorAxis = MaxX - MinX;
    m_SemiMinorAxis = MaxY - MinY;

    if (m_SemiMinorAxis > m_SemiMajorAxis)
        {
        double TempDistance = m_SemiMajorAxis;

        m_IsVertical = true;

        m_SemiMajorAxis = m_SemiMinorAxis;
        m_SemiMinorAxis = TempDistance;

        m_Center.SetX((m_SemiMinorAxis / 2.0) + MinX);
        m_Center.SetY((m_SemiMajorAxis / 2.0) + MinY);

        double C = sqrt(pow(m_SemiMajorAxis / 2.0, 2.0) -
                        pow(m_SemiMinorAxis / 2.0, 2.0));

        m_F1.SetX(m_Center.GetX());
        m_F1.SetY(-C + m_Center.GetY());

        m_F2.SetX(m_Center.GetX());
        m_F2.SetY(C + m_Center.GetY());
        }
    else
        {
        double C = sqrt(pow(m_SemiMajorAxis / 2.0, 2.0) -
                         pow(m_SemiMinorAxis / 2.0, 2.0));

        m_Center.SetX((m_SemiMajorAxis / 2.0) + MinX);
        m_Center.SetY((m_SemiMinorAxis / 2.0) + MinY);

        m_IsVertical = false;

        m_F1.SetX(-C + m_Center.GetX());
        m_F1.SetY(m_Center.GetY());

        m_F2.SetX(C + m_Center.GetX());
        m_F2.SetY(m_Center.GetY());
        }

    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DEllipse object.
//-----------------------------------------------------------------------------
inline HVE2DEllipse::HVE2DEllipse(const HVE2DEllipse& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_IsVertical(pi_rObj.m_IsVertical),
      m_Center(pi_rObj.m_Center),
      m_F1(pi_rObj.m_F1),
      m_F2(pi_rObj.m_F2),
      m_SemiMinorAxis(pi_rObj.m_SemiMinorAxis),
      m_SemiMajorAxis(pi_rObj.m_SemiMajorAxis)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DEllipse::~HVE2DEllipse()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DEllipse& HVE2DEllipse::operator=(const HVE2DEllipse& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Invoque ancester copy
        HVE2DSimpleShape::operator=(pi_rObj);

        // Copy attributes
        m_F1            = pi_rObj.m_F1;
        m_F2            = pi_rObj.m_F2;
        m_SemiMajorAxis = pi_rObj.m_SemiMajorAxis;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method indicates if the ellipse is empty
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::IsEmpty() const
    {
    return (m_SemiMajorAxis == 0.0) ||
           (m_SemiMinorAxis == 0.0);
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DEllipse::GetShapeType() const
    {
    return(HVE2DEllipse::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method returns the ellipse's area
//-----------------------------------------------------------------------------
inline double HVE2DEllipse::CalculateArea() const
    {
    return (PI * (m_SemiMajorAxis / 2) * (m_SemiMinorAxis / 2));
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method returns the ellipse's perimeter
//-----------------------------------------------------------------------------
inline double HVE2DEllipse::CalculatePerimeter() const
    {
    double A = m_SemiMajorAxis / 2;
    double B = m_SemiMinorAxis / 2;
    double H = pow((A - B) / (A + B), 2);

    return PI * (A + B) * (1 + (H / 4) + (pow(H, 2) / 64) + (pow(H, 3) / 256));
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method indicates if given point is in the ellipse
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    // Set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    double R1PlusR2 = (m_F1 - pi_rPoint).CalculateLength() + (m_F2 - pi_rPoint).CalculateLength();

    return((R1PlusR2 < m_SemiMajorAxis) && (!HDOUBLE_EQUAL(R1PlusR2, m_SemiMajorAxis, Tolerance)));
    }

//-----------------------------------------------------------------------------
// IsPointOn
// This method indicates if given point is on the ellipse
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::IsPointOn(const HGF2DLocation& pi_rPoint,
                                     HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                     double                          pi_Tolerance) const

    {
    double R1PlusR2 = (m_F1 - pi_rPoint).CalculateLength() + (m_F2 - pi_rPoint).CalculateLength();

    return ((pi_Tolerance == HVE_USE_INTERNAL_EPSILON) ?
            HDOUBLE_EQUAL(R1PlusR2, m_SemiMajorAxis, GetTolerance()) :
            HDOUBLE_EQUAL(R1PlusR2, m_SemiMajorAxis, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method indicates if given point is on ellipse
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                        HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                        double                          pi_Tolerance) const
    {
    // The point must be in the same coordinate system as the ellipse
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    double R1PlusR2 = (m_F1 - pi_rPoint).CalculateLength() + (m_F2 - pi_rPoint).CalculateLength();

    return ((pi_Tolerance == HVE_USE_INTERNAL_EPSILON) ?
            HDOUBLE_EQUAL(R1PlusR2, m_SemiMajorAxis, GetTolerance()) :
            HDOUBLE_EQUAL(R1PlusR2, m_SemiMajorAxis, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// MakeEmpty
// This method makes the ellipse empty
//-----------------------------------------------------------------------------
inline void HVE2DEllipse::MakeEmpty()
    {
    m_SemiMajorAxis = 0.0;
    m_SemiMinorAxis = 0.0;
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the ellipse's extent
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DEllipse::GetExtent() const
    {
    HGF2DExtent Extent(GetCoordSys());

    if (m_IsVertical)
        {
        Extent.SetXMin(m_Center.GetX() - m_SemiMinorAxis / 2.0);
        Extent.SetXMax(m_Center.GetX() + m_SemiMinorAxis / 2.0);
        Extent.SetYMin(m_Center.GetY() - m_SemiMajorAxis / 2.0);
        Extent.SetYMax(m_Center.GetY() + m_SemiMajorAxis / 2.0);
        }
    else
        {
        Extent.SetXMin(m_Center.GetX() - m_SemiMajorAxis / 2.0);
        Extent.SetXMax(m_Center.GetX() + m_SemiMajorAxis / 2.0);
        Extent.SetYMin(m_Center.GetY() - m_SemiMinorAxis / 2.0);
        Extent.SetYMax(m_Center.GetY() + m_SemiMinorAxis / 2.0);
        }

    return Extent;
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the ellipse is adjacent with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    return(GetLinear().AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the ellipse is contiguous with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return(GetLinear().AreContiguous(pi_rVector));
    }

//-----------------------------------------------------------------------------
// Clone
// This method returns a clone of ellipse
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DEllipse::Clone() const
    {
    return(new HVE2DEllipse(*this));
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DEllipse::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    return (HVE2DPolygon(GetLinear()).CalculateClosestPoint(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::Crosses(const HVE2DVector& pi_rVector) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // Since crossing computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygon(GetLinear()).Crosses(pi_rVector));
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
inline size_t HVE2DEllipse::Intersect(const HVE2DVector& pi_rVector,
                                      HGF2DLocationCollection* po_pCrossPoints) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // Since intersect computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygon(GetLinear()).Intersect(pi_rVector, po_pCrossPoints));
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the ellipse is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
inline bool HVE2DEllipse::AreContiguousAt(const HVE2DVector& pi_rVector,
                                           const HGF2DLocation& pi_rPoint) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    return(GetLinear().AreContiguousAt(pi_rVector, pi_rPoint));
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between ellipse and vector
//-----------------------------------------------------------------------------
inline void HVE2DEllipse::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                       const HGF2DLocation& pi_rPoint,
                                                       HGF2DLocation* po_pFirstContiguousnessPoint,
                                                       HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Create a polygon and process by it
    HVE2DPolygon(GetLinear()).ObtainContiguousnessPointsAt(pi_rVector,
                                                           pi_rPoint,
                                                           po_pFirstContiguousnessPoint,
                                                           po_pSecondContiguousnessPoint);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between ellipse and vector
//-----------------------------------------------------------------------------
inline size_t HVE2DEllipse::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    HPRECONDITION(AreContiguous(pi_rVector));

    return (HVE2DPolygon(GetLinear()).ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints));
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of ellipse at specified point
//-----------------------------------------------------------------------------
inline double HVE2DEllipse::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                         HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    HASSERT(0); //Not coded yet

    return 0;
    /*Ellipse
        // The acceleration on a ellipse is a constant
        return (pi_Direction == HVE2DVector::ALPHA ? 1.0 / m_Radius : -1.0 / m_Radius);
                                                      */
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of ellipse at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HVE2DEllipse::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                 HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return((m_Center - pi_rPoint).CalculateBearing() +
           (pi_Direction == HVE2DVector::ALPHA ? PI/2 : -PI/2));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the circle by specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DEllipse::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    m_Center += pi_rDisplacement;
    m_F1 += pi_rDisplacement;
    m_F2 += pi_rDisplacement;

    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the circle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
inline void HVE2DEllipse::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Change values
    m_Center += ((m_Center - pi_rScaleOrigin) * pi_ScaleFactor);
    m_F1     += ((m_Center - pi_rScaleOrigin) * pi_ScaleFactor);
    m_F2     += ((m_Center - pi_rScaleOrigin) * pi_ScaleFactor);

    m_SemiMajorAxis *= pi_ScaleFactor;
    m_SemiMinorAxis *= pi_ScaleFactor;

    ResetTolerance();
    }

END_IMAGEPP_NAMESPACE

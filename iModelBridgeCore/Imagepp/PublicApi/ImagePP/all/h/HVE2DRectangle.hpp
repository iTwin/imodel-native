//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DRectangle.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle()
    : HVE2DSimpleShape(),
      m_XMin(0.0),
      m_YMin(0.0),
      m_XMax(0.0),
      m_YMax(0.0)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle (double                      pi_XMin,
                                       double                      pi_YMin,
                                       double                      pi_XMax,
                                       double                      pi_YMax,
                                       const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_XMin(pi_XMin),
      m_YMin(pi_YMin),
      m_XMax(pi_XMax),
      m_YMax(pi_YMax)
    {
    HPRECONDITION(pi_XMin <= pi_XMax);
    HPRECONDITION(pi_YMin <= pi_YMax);

    // Set tolerance if necessary
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Constructor from rectangle fence
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle(const HGF2DRectangle&               pi_rRect,
                                      const HFCPtr<HGF2DCoordSys>&        pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys)
    {
    // Check that  the fence is valid
    HPRECONDITION(pi_rRect.GetXMin() <= pi_rRect.GetXMax());
    HPRECONDITION(pi_rRect.GetYMin() <= pi_rRect.GetYMax());

    // Copy fence values to rectangle
    m_XMin = pi_rRect.GetXMin();
    m_YMin = pi_rRect.GetYMin();
    m_XMax = pi_rRect.GetXMax();
    m_YMax = pi_rRect.GetYMax();

    // Set tolerance if necessary
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle (const HGF2DExtent& pi_rExtent)
    : HVE2DSimpleShape(pi_rExtent.GetCoordSys())
// Disabled because of an internal compiler error (VC50)
//  m_XMin(pi_rExtent.GetXMin()),
//  m_YMin(pi_rExtent.GetYMin()),
//  m_XMax(pi_rExtent.GetXMax()),
//  m_YMax(pi_rExtent.GetYMax())
    {
    // Extent provided must be defined
    HPRECONDITION(pi_rExtent.IsDefined());

    m_XMin = pi_rExtent.GetXMin();
    m_XMax = pi_rExtent.GetXMax();
    m_YMin = pi_rExtent.GetYMin();
    m_YMax = pi_rExtent.GetYMax();

    // Set tolerance if necessary
    ResetTolerance();

    HPOSTCONDITION(m_XMin <= m_XMax);
    HPOSTCONDITION(m_YMin <= m_YMax);
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle(const HGF2DLocation& pi_rFirstPoint,
                                      const HGF2DLocation& pi_rSecondPoint)
    : HVE2DSimpleShape(pi_rFirstPoint.GetCoordSys())
// The following cause INTERNAL VC++ ERROR
//: HVE2DSimpleShape(pi_rFirstPoint.GetCoordSys()),
//  m_XMin(MIN(pi_rFirstPoint.GetX(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetX())),
//  m_XMax(MAX(pi_rFirstPoint.GetX(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetX())),
//  m_YMin(MIN(pi_rFirstPoint.GetY(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetY())),
//  m_YMax(MAX(pi_rFirstPoint.GetY(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetY()))
    {
    if (pi_rFirstPoint.GetCoordSys() != pi_rSecondPoint.GetCoordSys())
        {
        m_XMin = MIN(pi_rFirstPoint.GetX(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetX());
        m_XMax = MAX(pi_rFirstPoint.GetX(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetX());
        m_YMin = MIN(pi_rFirstPoint.GetY(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetY());
        m_YMax = MAX(pi_rFirstPoint.GetY(), pi_rSecondPoint.ExpressedIn(pi_rFirstPoint.GetCoordSys()).GetY());
        }
    else
        {
        m_XMin = MIN(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
        m_XMax = MAX(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
        m_YMin = MIN(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());
        m_YMax = MAX(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());
        }

    // Set tolerance if necessary
    ResetTolerance();

    HPOSTCONDITION(m_XMin <= m_XMax);
    HPOSTCONDITION(m_YMin <= m_YMax);
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_XMin(0.0),
      m_YMin(0.0),
      m_XMax(0.0),
      m_YMax(0.0)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DRectangle object.
//-----------------------------------------------------------------------------
inline HVE2DRectangle::HVE2DRectangle(const HVE2DRectangle& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_XMin(pi_rObj.m_XMin),
      m_XMax(pi_rObj.m_XMax),
      m_YMin(pi_rObj.m_YMin),
      m_YMax(pi_rObj.m_YMax)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DRectangle::~HVE2DRectangle()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HVE2DRectangle& HVE2DRectangle::operator=(const HVE2DRectangle& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Call ancester operator
        HVE2DSimpleShape::operator=(pi_rObj);

        // Copy attributes
        m_XMin = pi_rObj.m_XMin;
        m_XMax = pi_rObj.m_XMax;
        m_YMin = pi_rObj.m_YMin;
        m_YMax = pi_rObj.m_YMax;
        }

    // Return reference to self
    return (*this);
    }

/** -----------------------------------------------------------------------------
    These methods extract the spatial information of the rectangle.

    @param po_pMinPoint Pointer to an HGF2DLocation object that receives
                        the origin of the rectangle.

    @param po_pMaxPoint Pointer to HGF2DLocation object that receives
                        the corner of the rectangle.

    Example
    @code
    @end
    -----------------------------------------------------------------------------
*/
inline void HVE2DRectangle::GetRectangle(HGF2DLocation* po_pMinPoint,
                                         HGF2DLocation* po_pMaxPoint) const
    {
    // Check that recipient variables are provided
    HPRECONDITION(po_pMinPoint != 0);
    HPRECONDITION(po_pMaxPoint != 0);

    // Extract points
    po_pMinPoint->SetCoordSys(GetCoordSys());
    po_pMinPoint->SetX(m_XMin);
    po_pMinPoint->SetY(m_YMin);
    po_pMaxPoint->SetCoordSys(GetCoordSys());
    po_pMaxPoint->SetX(m_XMax);
    po_pMaxPoint->SetY(m_YMax);
    }

/** -----------------------------------------------------------------------------
    These methods extract the spatial information of the rectangle.

    @param po_pXMin Pointer to double that receives the minimum X
                    of rectangle. The value must be interpreted in
                    the X dimension unit of the rectangle coordinate system.

    @param po_pYMin Pointer to double that receives the minimum Y
                    of rectangle. The value must be interpreted in
                    the Y dimension unit of the rectangle coordinate system.

    @param po_pXMax Pointer to double that receives the maximum X
                    of rectangle. The value must be interpreted in
                    the X dimension unit of the rectangle coordinate system.

    @param po_pYMax Pointer to double that receives the maximum X
                    of rectangle. The value must be interpreted in
                    the Y dimension unit of the rectangle coordinate system.

    Example
    @code
    @end
    -----------------------------------------------------------------------------
*/
inline void HVE2DRectangle::GetRectangle(double* po_pXMin,
                                         double* po_pYMin,
                                         double* po_pXMax,
                                         double* po_pYMax) const
    {
    // Check that recipient variables are provided
    HPRECONDITION(po_pXMin != 0);
    HPRECONDITION(po_pYMin != 0);
    HPRECONDITION(po_pXMax != 0);
    HPRECONDITION(po_pYMax != 0);


    // Extract minima and maxima
    *po_pXMin = m_XMin;
    *po_pYMin = m_YMin;
    *po_pXMax = m_XMax;
    *po_pYMax = m_YMax;
    }


/** -----------------------------------------------------------------------------
This method sets the spatial information of the rectangle.

@param pi_XMin Pointer to double that receives the minimum X
of rectangle. The value must be interpreted in
the X dimension unit of the rectangle coordinate system.

@param pi_YMin Pointer to double that receives the minimum Y
of rectangle. The value must be interpreted in
the Y dimension unit of the rectangle coordinate system.

@param pi_XMax Pointer to double that receives the maximum X
of rectangle. The value must be interpreted in
the X dimension unit of the rectangle coordinate system.

@param pi_YMax Pointer to double that receives the maximum X
of rectangle. The value must be interpreted in
the Y dimension unit of the rectangle coordinate system.

Example
@code
@end
-----------------------------------------------------------------------------
*/
inline void HVE2DRectangle::SetRectangle(double pi_XMin,
                                         double pi_YMin,
                                         double pi_XMax,
                                         double pi_YMax)
    {
    // Check that recipient variables are provided
    HPRECONDITION(pi_XMin <= pi_XMax);
    HPRECONDITION(pi_YMin <= pi_YMax);

    // Extract minima and maxima
    m_XMin = pi_XMin;
    m_YMin = pi_YMin;
    m_XMax = pi_XMax;
    m_YMax = pi_YMax;

    }


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the rectangle boundary
//-----------------------------------------------------------------------------
inline bool HVE2DRectangle::IsPointOn(const HGF2DLocation&             pi_rPoint,
                                       HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                       double                          pi_Tolerance) const
    {
    return(IsPointOnSCS(HGF2DLocation(pi_rPoint,GetCoordSys()), pi_ExtremityProcessing, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the rectangle boundary
//-----------------------------------------------------------------------------
inline bool HVE2DRectangle::IsPointOnSCS(const HGF2DLocation&             pi_rPoint,
                                          HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                          double                          pi_Tolerance) const
    {
    // The rectangle and point must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    // Extract raw values
    double X(pi_rPoint.GetX());
    double Y(pi_rPoint.GetY());

    // Extract tolerance if internal tolerance must be use obtain it
    double Tolerance = pi_Tolerance;
    if (Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // The result tolerance may not be negative
    HASSERT(Tolerance >= 0.0);

    // The second parameter is ignored since a shape has no extremities
    return((HDOUBLE_EQUAL(X, m_XMin, Tolerance) && Y <= m_YMax && Y >= m_YMin) ||
           (HDOUBLE_EQUAL(X, m_XMax, Tolerance) && Y <= m_YMax && Y >= m_YMin) ||
           (HDOUBLE_EQUAL(Y, m_YMin, Tolerance) && X <= m_XMax && X >= m_XMin) ||
           (HDOUBLE_EQUAL(Y, m_YMax, Tolerance) && X <= m_XMax && X >= m_XMin) ||
           (HDOUBLE_EQUAL(X, m_XMin, Tolerance) &&
            (HDOUBLE_EQUAL(Y, m_YMin, Tolerance) || HDOUBLE_EQUAL(Y, m_YMax, Tolerance))) ||
           (HDOUBLE_EQUAL(X, m_XMax, Tolerance) &&
            (HDOUBLE_EQUAL(Y, m_YMin, Tolerance) || HDOUBLE_EQUAL(Y, m_YMax, Tolerance))));
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DRectangle::Clone() const
    {
    return (new HVE2DRectangle(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DRectangle::GetExtent() const
    {
    // Return correct extent depending on emptyness
    return((IsEmpty() ? HGF2DExtent(GetCoordSys()) :
            HGF2DExtent(m_XMin, m_YMin, m_XMax, m_YMax, GetCoordSys())));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the rectangle by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DRectangle::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // We calculate the displacement deltas as expressed in the unit of the
    // coordinate system of the rectangle
    double     DeltaX(pi_rDisplacement.GetDeltaX());
    double     DeltaY(pi_rDisplacement.GetDeltaY());

    // We apply the deltas to every coordinate of the rectangle
    m_XMin += DeltaX;
    m_YMin += DeltaY;
    m_XMax += DeltaX;
    m_YMax += DeltaY;

    // Set tolerance if necessary
    ResetTolerance();

    HASSERT(m_XMax >= m_XMin);
    HASSERT(m_YMax >= m_YMin);
    }



//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the rectangle is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DRectangle::IsEmpty() const
    {
    return(HDOUBLE_GREATER_OR_EQUAL(m_XMin, m_XMax, GetTolerance()) ||
           HDOUBLE_GREATER_OR_EQUAL(m_YMin, m_YMax, GetTolerance()));
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DRectangle::CalculateArea() const
    {
    return ((m_YMax - m_YMin) * (m_XMax - m_XMin));
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DRectangle::CalculatePerimeter() const
    {
    return((2 * (m_XMax - m_XMin)) + (2 * (m_YMax - m_YMin)));
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the rectangle area
//-----------------------------------------------------------------------------
inline bool HVE2DRectangle::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    HGF2DLocation   MyTestPoint(pi_rPoint, GetCoordSys());

    // Set Tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // The result tolerance may not be negative
    HASSERT(Tolerance >= 0.0);

    double     X(MyTestPoint.GetX());
    double     Y(MyTestPoint.GetY());

    return(HDOUBLE_SMALLER(X, m_XMax, Tolerance) &&
           HDOUBLE_GREATER(X, m_XMin, Tolerance) &&
           HDOUBLE_SMALLER(Y, m_YMax, Tolerance) &&
           HDOUBLE_GREATER(Y, m_YMin, Tolerance));
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the rectangle
//-----------------------------------------------------------------------------
inline void HVE2DRectangle::MakeEmpty()
    {
    m_XMin = m_XMax;
    m_YMin = m_YMax;
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DRectangle::GetShapeType() const
    {
    return(HVE2DRectangle::CLASS_ID);
    }

END_IMAGEPP_NAMESPACE

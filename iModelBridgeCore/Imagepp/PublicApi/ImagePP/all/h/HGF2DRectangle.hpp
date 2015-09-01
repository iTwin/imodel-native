//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DRectangle.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DRectangle::HGF2DRectangle()
    : HGF2DSimpleShape(),
      m_XMin(0.0),
      m_YMin(0.0),
      m_XMax(0.0),
      m_YMax(0.0)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGF2DRectangle::HGF2DRectangle (double                      pi_XMin,
                                       double                      pi_YMin,
                                       double                      pi_XMax,
                                       double                      pi_YMax)
    : HGF2DSimpleShape(),
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
// Constructor
//-----------------------------------------------------------------------------
inline HGF2DRectangle::HGF2DRectangle (const HGF2DLiteExtent& pi_rExtent)
    : HGF2DSimpleShape()
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
inline HGF2DRectangle::HGF2DRectangle(const HGF2DPosition& pi_rFirstPoint,
                                  const HGF2DPosition& pi_rSecondPoint)
    : HGF2DSimpleShape()
    {
    m_XMin = MIN(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
    m_XMax = MAX(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
    m_YMin = MIN(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());
    m_YMax = MAX(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());

    // Set tolerance if necessary
    ResetTolerance();

    HPOSTCONDITION(m_XMin <= m_XMax);
    HPOSTCONDITION(m_YMin <= m_YMax);
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DRectangle object.
//-----------------------------------------------------------------------------
inline HGF2DRectangle::HGF2DRectangle(const HGF2DRectangle& pi_rObj)
    : HGF2DSimpleShape(pi_rObj),
      m_XMin(pi_rObj.m_XMin),
      m_XMax(pi_rObj.m_XMax),
      m_YMin(pi_rObj.m_YMin),
      m_YMax(pi_rObj.m_YMax)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DRectangle::~HGF2DRectangle()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
inline HGF2DRectangle& HGF2DRectangle::operator=(const HGF2DRectangle& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Call ancester operator
        HGF2DSimpleShape::operator=(pi_rObj);

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
    @return The lower X value of rectangle
    @see GetXMax()
    @see GetYMin()
    @see GetYMax()
    ----------------------------------------------------------------------------- 
*/
 inline double HGF2DRectangle::GetXMin() const
{
    return(m_XMin);
}

/** -----------------------------------------------------------------------------
    @return The upper X value of rectangle
    @see GetXMin()
    @see GetYMin()
    @see GetYMax()
    ----------------------------------------------------------------------------- 
*/
inline double HGF2DRectangle::GetXMax() const
{
    return(m_XMax);
}

/** -----------------------------------------------------------------------------
    @return The lower Y value of rectangle
    @see GetXMax()
    @see GetXMin()
    @see GetYMax()
    ----------------------------------------------------------------------------- 
*/
inline double HGF2DRectangle::GetYMin() const
{
    return(m_YMin);
}

/** -----------------------------------------------------------------------------
    @return The upper Y value of rectangle
    @see GetXMax()
    @see GetYMin()
    @see GetXMin()
    ----------------------------------------------------------------------------- 
*/
inline double HGF2DRectangle::GetYMax() const
{
    return(m_YMax);
}

/** -----------------------------------------------------------------------------
    These methods extract the spatial information of the rectangle.

    @param po_pMinPoint Pointer to an HGF2DPosition object that receives
                        the origin of the rectangle.

    @param po_pMaxPoint Pointer to HGF2DPosition object that receives
                        the corner of the rectangle.

    -----------------------------------------------------------------------------
*/
inline void HGF2DRectangle::GetRectangle(HGF2DPosition* po_pMinPoint,
                                       HGF2DPosition* po_pMaxPoint) const
    {
    // Check that recipient variables are provided
    HPRECONDITION(po_pMinPoint != 0);
    HPRECONDITION(po_pMaxPoint != 0);

    // Extract points
    po_pMinPoint->SetX(m_XMin);
    po_pMinPoint->SetY(m_YMin);
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
inline void HGF2DRectangle::GetRectangle(double* po_pXMin,
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
inline void HGF2DRectangle::SetRectangle(double pi_XMin,
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
inline bool HGF2DRectangle::IsPointOn(const HGF2DPosition&             pi_rPoint,
                                    HGF2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                    double                          pi_Tolerance) const
    {
    // Extract raw values
    double X(pi_rPoint.GetX());
    double Y(pi_rPoint.GetY());

    // Extract tolerance if internal tolerance must be use obtain it
    double Tolerance = pi_Tolerance;
    if (Tolerance == HGF_USE_INTERNAL_EPSILON)
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
inline HGF2DVector* HGF2DRectangle::Clone() const
    {
    return (new HGF2DRectangle(*this));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DRectangle::GetExtent() const
    {
    // Return correct extent depending on emptyness
    return((IsEmpty() ? HGF2DLiteExtent() :
            HGF2DLiteExtent(m_XMin, m_YMin, m_XMax, m_YMax)));
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the rectangle by the specified displacement
//-----------------------------------------------------------------------------
inline void HGF2DRectangle::Move(const HGF2DDisplacement& pi_rDisplacement)
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
inline bool HGF2DRectangle::IsEmpty() const
    {
    return(HDOUBLE_GREATER_OR_EQUAL(m_XMin, m_XMax, GetTolerance()) ||
           HDOUBLE_GREATER_OR_EQUAL(m_YMin, m_YMax, GetTolerance()));
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HGF2DRectangle::CalculateArea() const
    {
    return ((m_YMax - m_YMin) * (m_XMax - m_XMin));
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HGF2DRectangle::CalculatePerimeter() const
    {
    return ((2 * (m_XMax - m_XMin)) + (2 * (m_YMax - m_YMin)));
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the rectangle area
//-----------------------------------------------------------------------------
inline bool HGF2DRectangle::IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const
    {
    // Set Tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HGF_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // The result tolerance may not be negative
    HASSERT(Tolerance >= 0.0);

    double     X = pi_rPoint.GetX();
    double     Y = pi_rPoint.GetY();

    return(HDOUBLE_SMALLER(X, m_XMax, Tolerance) &&
           HDOUBLE_GREATER(X, m_XMin, Tolerance) &&
           HDOUBLE_SMALLER(Y, m_YMax, Tolerance) &&
           HDOUBLE_GREATER(Y, m_YMin, Tolerance));
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the rectangle
//-----------------------------------------------------------------------------
inline void HGF2DRectangle::MakeEmpty()
    {
    m_XMin = m_XMax;
    m_YMin = m_YMax;
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DRectangle::GetShapeType() const
    {
    return(HGF2DRectangle::CLASS_ID);
    }


END_IMAGEPP_NAMESPACE
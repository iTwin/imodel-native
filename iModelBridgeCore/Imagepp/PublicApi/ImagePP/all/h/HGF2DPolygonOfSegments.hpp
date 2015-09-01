//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPolygonOfSegments.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DPolygonOfSegments::HGF2DPolygonOfSegments()
    : HGF2DSimpleShape(),
      m_RotationDirectionUpToDate(false),
      m_RotationDirection(CW),
      m_PolySegment()
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DPolygonOfSegments object.
//-----------------------------------------------------------------------------
inline HGF2DPolygonOfSegments::HGF2DPolygonOfSegments(const HGF2DPolygonOfSegments& pi_rObj)
    : HGF2DSimpleShape(pi_rObj),
      m_PolySegment(pi_rObj.m_PolySegment),
      m_RotationDirectionUpToDate(pi_rObj.m_RotationDirectionUpToDate),
      m_RotationDirection(pi_rObj.m_RotationDirection)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DPolygonOfSegments::~HGF2DPolygonOfSegments()
    {
    HINVARIANTS;

    // Clear list of points
    m_PolySegment.m_Points.clear();
    }


//-----------------------------------------------------------------------------
// GetListOfPoints
// This method returns the list of points composing the polygon of segmetns
//-----------------------------------------------------------------------------
inline const HGF2DPositionCollection& HGF2DPolygonOfSegments::GetListOfPoints() const
    {
    HINVARIANTS;

    return m_PolySegment.m_Points;
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns the complex linear of the shape in the specified
// rotation direction
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DLinear> HGF2DPolygonOfSegments::GetLinear(HGF2DSimpleShape::RotationDirection pi_RotationDirection) const
    {
    HINVARIANTS;

    HFCPtr<HGF2DLinear>  NewComplex = GetLinear();

    // Check if the current rotation direction is appropriate
    if (CalculateRotationDirection() != pi_RotationDirection)
        {
        // The linear does not rotate in the proper direction
        // Reverse the copy
        NewComplex->Reverse();
        }

    return(NewComplex);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DPolygonOfSegments::Clone() const
    {
    HINVARIANTS;

    return (new HGF2DPolygonOfSegments(*this));
    }



//-----------------------------------------------------------------------------
// Move
// This method moves the polygon by the specified displacement
//-----------------------------------------------------------------------------
inline void HGF2DPolygonOfSegments::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HINVARIANTS;

    m_PolySegment.Move(pi_rDisplacement);
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HGF2DPolygonOfSegments::IsEmpty() const
    {
    HINVARIANTS;

    // An empty polygon has less than 3 points AND has a non null extent
    HGF2DLiteExtent MyExtent(GetExtent());

    return((m_PolySegment.GetSize() <= 3) || 
           HNumeric<double>::EQUAL(MyExtent.GetWidth(), 0.0, GetTolerance()) || 
           HNumeric<double>::EQUAL(MyExtent.GetHeight() ,0.0, GetTolerance()));
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HGF2DPolygonOfSegments::CalculateArea() const
    {
    HINVARIANTS;

    // Calculate signed area and return absolute value
    return fabs(CalculateRawArea());
    }


/** -----------------------------------------------------------------------------
    This method calculates the rotation direction of the polygon of segments.

    @return HGF2DSimpleShape::CW if the polygon is defined Clock-Wise and
            HGF2DSimpleShape::CCW otherwise, including if the polygon is null.

    Example
    @code
    @end
    -----------------------------------------------------------------------------
*/
inline HGF2DSimpleShape::RotationDirection HGF2DPolygonOfSegments::CalculateRotationDirection() const
    {
    HINVARIANTS;

    if (!m_RotationDirectionUpToDate)
        {
        // Cast out of const
        HGF2DSimpleShape::RotationDirection*   pTheDirection = (HGF2DSimpleShape::RotationDirection*)&m_RotationDirection;

        *pTheDirection = (CalculateRawArea() < 0.0 ? HGF2DSimpleShape::CW : HGF2DSimpleShape::CCW);

        (*((bool*)&m_RotationDirectionUpToDate)) = true;
        }

    return(m_RotationDirection);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HGF2DPolygonOfSegments::CalculatePerimeter() const
    {
    HINVARIANTS;

    return(m_PolySegment.CalculateLength());
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HGF2DPolygonOfSegments::MakeEmpty()
    {
    HINVARIANTS;

    m_PolySegment.MakeEmpty();

    HGF2DVector::SetTolerance(m_PolySegment.GetTolerance());

    }

//-----------------------------------------------------------------------------
// InteractsWith
// PRIVATE METHOD
// This method analyses the two given polygons of segments, and in the case there
// is any interaction points, produces copies of the points of the polygons
// to which have been added all interaction points, in the direction
// appropriate for super scanning.
// It returns true if polygons are contiguous or intersect
//-----------------------------------------------------------------------------
inline bool HGF2DPolygonOfSegments::InteractsWith(const HGF2DPolygonOfSegments& pi_rPolygon,
                                                   HGF2DPositionCollection* po_pSelfPolyPoints,
                                                   HGF2DPositionCollection* po_pGivenPolyPoints,
                                                   HGF2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                                   bool pi_IgnoreSimpleContiguousness,
                                                   bool* po_pContiguousInteraction) const
    {
    HINVARIANTS;

    // None of the two shape may be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HPRECONDITION(po_pSelfPolyPoints != 0);
    HPRECONDITION(po_pGivenPolyPoints != 0);


    // Check if X and Y units are identical
    return(InteractsWithSameUnits(pi_rPolygon, po_pSelfPolyPoints, po_pGivenPolyPoints, pi_Operation, pi_IgnoreSimpleContiguousness, po_pContiguousInteraction));
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPosition
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HGF2DShape::SpatialPosition HGF2DPolygonOfSegments::CalculateSpatialPositionOfPosition(const HGF2DPosition& pi_rPoint,
        double pi_Tolerance) const
    {
    HINVARIANTS;

    return(CalculateSpatialPositionOfPositionSameUnits(pi_rPoint, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DPolygonOfSegments::GetShapeType() const
    {
    HINVARIANTS;

    return(HGF2DPolygonOfSegments::CLASS_ID);
    }


END_IMAGEPP_NAMESPACE

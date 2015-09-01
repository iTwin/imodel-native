//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DPolygonOfSegments.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DPolygonOfSegments::HVE2DPolygonOfSegments()
    : HVE2DSimpleShape(),
      m_RotationDirectionUpToDate(false),
      m_RotationDirection(CW),
      m_PolySegment(GetCoordSys())
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Constructor with a coordinate system only
//-----------------------------------------------------------------------------
inline HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_RotationDirectionUpToDate(false),
      m_RotationDirection(CW),
      m_PolySegment(pi_rpCoordSys)
    {
    HINVARIANTS;
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DPolygonOfSegments object.
//-----------------------------------------------------------------------------
inline HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HVE2DPolygonOfSegments& pi_rObj)
    : HVE2DSimpleShape(pi_rObj),
      m_PolySegment(pi_rObj.m_PolySegment),
      m_RotationDirectionUpToDate(pi_rObj.m_RotationDirectionUpToDate),
      m_RotationDirection(pi_rObj.m_RotationDirection)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DPolygonOfSegments::~HVE2DPolygonOfSegments()
    {
    HINVARIANTS;

    // Clear list of points
    m_PolySegment.m_Points.clear();
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns the complex linear of the shape in the specified
// rotation direction
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear* HVE2DPolygonOfSegments::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_RotationDirection) const
    {
    HINVARIANTS;

    HVE2DComplexLinear*  pReturnedLinear = new HVE2DComplexLinear(GetLinear());

    if (CalculateRotationDirection() != pi_RotationDirection)
        // Reverse the copy
        pReturnedLinear->Reverse();

    return(pReturnedLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns the complex linear of the shape in the specified
// rotation direction
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear HVE2DPolygonOfSegments::GetLinear(HVE2DSimpleShape::RotationDirection pi_RotationDirection) const
    {
    HINVARIANTS;

    HVE2DComplexLinear  NewComplex(GetLinear());

    // Check if the current rotation direction is appropriate
    if (CalculateRotationDirection() != pi_RotationDirection)
        {
        // The linear does not rotate in the proper direction
        // Reverse the copy
        NewComplex.Reverse();
        }

    return(NewComplex);
    }



#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the polygon flirts with given vector.
// INCOMPLETE ???
//-----------------------------------------------------------------------------
inline bool HVE2DPolygonOfSegments::Flirts(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    // ???? What happens if crosses at start point ?
    return (m_ComplexLinear.Flirts(pi_rVector) || (m_ComplexLinear.ConnectsTo(pi_rVector)));
    }
#endif

//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DPolygonOfSegments::Clone() const
    {
    HINVARIANTS;

    return (new HVE2DPolygonOfSegments(*this));
    }



//-----------------------------------------------------------------------------
// Move
// This method moves the polygon by the specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DPolygonOfSegments::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HINVARIANTS;

    m_PolySegment.Move(pi_rDisplacement);
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method checks if the polygon is defined or not (represents an empty shape)
//-----------------------------------------------------------------------------
inline bool HVE2DPolygonOfSegments::IsEmpty() const
    {
    HINVARIANTS;

    // An empty polygon has less than 3 points AND has a non null extent
    HGF2DExtent MyExtent(GetExtent());

    return((m_PolySegment.GetSize() <= 3) ||
           HDOUBLE_EQUAL(MyExtent.GetWidth(), 0.0, GetTolerance()) ||
           HDOUBLE_EQUAL(MyExtent.GetHeight(), 0.0, GetTolerance()));
    }

//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the shape
//-----------------------------------------------------------------------------
inline double HVE2DPolygonOfSegments::CalculateArea() const
    {
    HINVARIANTS;

    // Calculate signed area and return absolute value
    return(fabs(CalculateRawArea()));
    }


/** -----------------------------------------------------------------------------
    This method calculates the rotation direction of the polygon of segments.

    @return HVE2DSimpleShape::CW if the polygon is defined Clock-Wise and
            HVE2DSimpleShape::CCW otherwise, including if the polygon is null.

    Example
    @code
    @end
    -----------------------------------------------------------------------------
*/
inline HVE2DSimpleShape::RotationDirection HVE2DPolygonOfSegments::CalculateRotationDirection() const
    {
    HINVARIANTS;

    if (!m_RotationDirectionUpToDate)
        {
        // Cast out of const
        HVE2DSimpleShape::RotationDirection*   pTheDirection = (HVE2DSimpleShape::RotationDirection*)&m_RotationDirection;

        *pTheDirection = (CalculateRawArea() < 0.0 ? HVE2DSimpleShape::CW : HVE2DSimpleShape::CCW);

        (*((bool*)&m_RotationDirectionUpToDate)) = true;
        }

    return(m_RotationDirection);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
inline double HVE2DPolygonOfSegments::CalculatePerimeter() const
    {
    HINVARIANTS;

    return(m_PolySegment.CalculateLength());
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polygon
//-----------------------------------------------------------------------------
inline void HVE2DPolygonOfSegments::MakeEmpty()
    {
    HINVARIANTS;

    m_PolySegment.MakeEmpty();

    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

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
inline bool HVE2DPolygonOfSegments::InteractsWith(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                   HGF2DPositionCollection* po_pSelfPolyPoints,
                                                   HGF2DPositionCollection* po_pGivenPolyPoints,
                                                   HVE2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                                   bool pi_IgnoreSimpleContiguousness,
                                                   bool* po_pContiguousInteraction) const
    {
    HINVARIANTS;

    // None of the two shape may be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    // The two polygons must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    HPRECONDITION(po_pSelfPolyPoints != 0);
    HPRECONDITION(po_pGivenPolyPoints != 0);


    // Check if X and Y units are identical
    return InteractsWithSameUnits(pi_rPolygon, po_pSelfPolyPoints, po_pGivenPolyPoints, pi_Operation, pi_IgnoreSimpleContiguousness, po_pContiguousInteraction);
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPosition
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
inline HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfPosition(const HGF2DPosition& pi_rPoint,
        double pi_Tolerance) const
    {
    HINVARIANTS;

    return CalculateSpatialPositionOfPositionSameUnits(pi_rPoint, pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HVE2DShapeTypeId HVE2DPolygonOfSegments::GetShapeType() const
    {
    HINVARIANTS;

    return(HVE2DPolygonOfSegments::CLASS_ID);
    }

END_IMAGEPP_NAMESPACE

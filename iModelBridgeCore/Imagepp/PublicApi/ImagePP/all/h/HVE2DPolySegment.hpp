//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::HVE2DPolySegment()
    : HVE2DBasicLinear(),
      m_ExtentUpToDate(false)
    {
    m_VolatilePeer = false;

    CreateEmptyPolySegmentPeer();
	
    // Make sure start and end points are identical
    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system only)
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::HVE2DPolySegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys),
      m_ExtentUpToDate(false),
      m_Extent(pi_rpCoordSys)
    {
    m_VolatilePeer = false;

    // Default tolerance is used

    CreateEmptyPolySegmentPeer();
	
    // Make sure start and end points are identical

    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DPolySegment object.
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::HVE2DPolySegment(const HGF2DLocation& pi_rFirstPoint,
                                          const HGF2DLocation& pi_rSecondPoint)
    : HVE2DBasicLinear(pi_rFirstPoint, pi_rSecondPoint),
      m_ExtentUpToDate(false),
      m_Extent(pi_rFirstPoint.GetCoordSys())
    {
    m_VolatilePeer = false;




    HGF2DPositionCollection points;
    points.push_back(pi_rFirstPoint.GetPosition());
    points.push_back(pi_rSecondPoint.ExpressedIn(GetCoordSys()).GetPosition());

    CreatePolySegmentPeer(points);



    // Set tolerance
    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DPolySegment object.
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::HVE2DPolySegment(const HVE2DPolySegment& pi_rObj)
    : HVE2DBasicLinear(pi_rObj),
      m_ExtentUpToDate(pi_rObj.m_ExtentUpToDate),
      m_Extent(pi_rObj.m_Extent)
    {

    m_VolatilePeer = false;

    CreateCopyPolySegmentPeer(pi_rObj.GetPolySegmentPeer());
    
    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::~HVE2DPolySegment()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polysegment.
//-----------------------------------------------------------------------------
inline HVE2DPolySegment& HVE2DPolySegment::operator=(const HVE2DPolySegment& pi_rObj)
    {
    // Check that given is not self
    if (&pi_rObj != this)
        {
        // Invoque the ancester operator
        // this copies start and end points
        HVE2DBasicLinear::operator=(pi_rObj);

        // Copy attributes
		CreateCopyPolySegmentPeer(pi_rObj.GetPolySegmentPeer());

        m_ExtentUpToDate = pi_rObj.m_ExtentUpToDate;
        m_Extent = pi_rObj.m_Extent;

        HINVARIANTS;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetPoint
// Returns the point designated by index
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DPolySegment::GetPoint(size_t pi_Index) const
    {
    HINVARIANTS;

    // The given index must be valid
    HPRECONDITION((pi_Index >= 0) && (pi_Index < GetPolySegmentPeer().GetSize()));

    return HGF2DLocation(GetPolySegmentPeer().GetPoint(pi_Index), GetCoordSys());
    }
//-----------------------------------------------------------------------------
// GetPosition
// Returns the point designated by index
//-----------------------------------------------------------------------------
inline HGF2DPosition HVE2DPolySegment::GetPosition(size_t pi_Index) const
    {
    // &&AR TO MANY HINVARIANTS;

    // The given index must be valid
    HPRECONDITION((pi_Index >= 0) && (pi_Index < GetPoints().size()));

    HPRECONDITION((pi_Index >= 0) && (pi_Index <  GetPolySegmentPeer().GetSize()));

    return GetPolySegmentPeer().GetPoint(pi_Index);
    
    }


//-----------------------------------------------------------------------------
// GetSize
// Returns the number of points in polysegment
//-----------------------------------------------------------------------------
inline size_t HVE2DPolySegment::GetSize() const
    {
   // &&AR Too many
//  HINVARIANTS;
    return GetPolySegmentPeer().GetSize();
    }

//-----------------------------------------------------------------------------
// GetBasicLinearType
// Return the basic linear type
//-----------------------------------------------------------------------------
inline HVE2DBasicLinearTypeId HVE2DPolySegment::GetBasicLinearType() const
    {
    HINVARIANTS;

    return (HVE2DPolySegment::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of polysegment at
// specified point
//-----------------------------------------------------------------------------
inline double HVE2DPolySegment::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                               HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;

    // The point must be located on polysegment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a polysegment is always 0.0
    return 0.0;
    }




//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the polysegment
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DPolySegment::Clone() const
    {
    HINVARIANTS;

    return(new HVE2DPolySegment(*this));
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the polysegment is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE2DPolySegment::IsNull() const
    {
    HINVARIANTS;
    return GetPolySegmentPeer().IsNull();
    }

END_IMAGEPP_NAMESPACE

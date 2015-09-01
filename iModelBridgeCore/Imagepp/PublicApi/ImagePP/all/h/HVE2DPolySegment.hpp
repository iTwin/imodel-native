//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DPolySegment.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    // Default tolerance is used

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
    // The start and end points are set ... all we need is to copy them\
    // to the positions list
    m_Points.push_back(m_StartPoint.GetPosition());
    m_Points.push_back(pi_rSecondPoint.ExpressedIn(GetCoordSys()).GetPosition());

    // Set tolerance
    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DPolySegment object.
//-----------------------------------------------------------------------------
inline HVE2DPolySegment::HVE2DPolySegment(const HVE2DPolySegment& pi_rObj)
    : HVE2DBasicLinear(pi_rObj),
      m_Points(pi_rObj.m_Points),
      m_ExtentUpToDate(pi_rObj.m_ExtentUpToDate),
      m_Extent(pi_rObj.m_Extent)
    {
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
        m_Points = pi_rObj.m_Points;
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
    HPRECONDITION((pi_Index >= 0) && (pi_Index < m_Points.size()));

    // Return point in self coordinate system
    return(HGF2DLocation(m_Points[pi_Index], GetCoordSys()));
    }
//-----------------------------------------------------------------------------
// GetPosition
// Returns the point designated by index
//-----------------------------------------------------------------------------
inline const HGF2DPosition& HVE2DPolySegment::GetPosition(size_t pi_Index) const
    {
    HINVARIANTS;

    // The given index must be valid
    HPRECONDITION((pi_Index >= 0) && (pi_Index < m_Points.size()));

    // Return point in self coordinate system
    return(m_Points[pi_Index]);
    }


//-----------------------------------------------------------------------------
// GetSize
// Returns the number of points in polysegment
//-----------------------------------------------------------------------------
inline size_t HVE2DPolySegment::GetSize() const
    {
    HINVARIANTS;

    return m_Points.size();
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

    return(m_Points.size() == 0);
    }

END_IMAGEPP_NAMESPACE

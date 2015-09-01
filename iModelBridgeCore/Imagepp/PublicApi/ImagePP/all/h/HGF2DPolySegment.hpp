//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPolySegment.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DPolySegment::HGF2DPolySegment()
    : HGF2DBasicLinear(),
      m_ExtentUpToDate(false)
    {
    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DPolySegment object.
//-----------------------------------------------------------------------------
inline HGF2DPolySegment::HGF2DPolySegment(const HGF2DPosition& pi_rFirstPoint,
                                      const HGF2DPosition& pi_rSecondPoint)
    : HGF2DBasicLinear(pi_rFirstPoint, pi_rSecondPoint),
      m_ExtentUpToDate(false),
      m_Extent()
    {
    // The start and end points are set ... all we need is to copy them\
    // to the positions list
    m_Points.push_back(m_StartPoint);
    m_Points.push_back(pi_rSecondPoint);

    // Set tolerance
    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DPolySegment object.
//-----------------------------------------------------------------------------
inline HGF2DPolySegment::HGF2DPolySegment(const HGF2DPolySegment& pi_rObj)
    : HGF2DBasicLinear(pi_rObj),
      m_Points(pi_rObj.m_Points),
      m_ExtentUpToDate(pi_rObj.m_ExtentUpToDate),
      m_Extent(pi_rObj.m_Extent)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DPolySegment::~HGF2DPolySegment()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polysegment.
//-----------------------------------------------------------------------------
inline HGF2DPolySegment& HGF2DPolySegment::operator=(const HGF2DPolySegment& pi_rObj)
    {
    // Check that given is not self
    if (&pi_rObj != this)
        {
        // Invoque the ancester operator
        // this copies start and end points
        HGF2DBasicLinear::operator=(pi_rObj);

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
inline HGF2DPosition HGF2DPolySegment::GetPoint(size_t pi_Index) const
    {
    HINVARIANTS;

    // The given index must be valid
    HPRECONDITION((pi_Index >= 0) && (pi_Index < m_Points.size()));

    // Return point in self coordinate system
    return m_Points[pi_Index];
    }



//-----------------------------------------------------------------------------
// GetSize
// Returns the number of points in polysegment
//-----------------------------------------------------------------------------
inline size_t HGF2DPolySegment::GetSize() const
    {
    HINVARIANTS;

    return m_Points.size();
    }

//-----------------------------------------------------------------------------
// GetBasicLinearType
// Return the basic linear type
//-----------------------------------------------------------------------------
inline HGF2DBasicLinearTypeId HGF2DPolySegment::GetBasicLinearType() const
    {
    HINVARIANTS;

    return (HGF2DPolySegment::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of polysegment at
// specified point
//-----------------------------------------------------------------------------
inline double
HGF2DPolySegment::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                             HGF2DVector::ArbitraryDirection pi_Direction) const
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
inline HGF2DVector* HGF2DPolySegment::Clone() const
    {
    HINVARIANTS;

    return(new HGF2DPolySegment(*this));
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the polysegment is null (no length)
//-----------------------------------------------------------------------------
inline bool HGF2DPolySegment::IsNull() const
    {
    HINVARIANTS;

    return(m_Points.size() == 0);
    }

END_IMAGEPP_NAMESPACE
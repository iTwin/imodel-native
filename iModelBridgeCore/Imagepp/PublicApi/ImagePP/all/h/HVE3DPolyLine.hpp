//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DPolyLine.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::HVE3DPolyLine(size_t i_rCapacity)
    : m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_AutoToleranceActive(true),
      m_Tolerance(HNumeric<double>::GLOBAL_EPSILON())
    {
    m_Points.reserve(i_rCapacity);
    ResetTolerance();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system only)
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::HVE3DPolyLine(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys,
                                    size_t                       i_rCapacity)
    : m_pCoordSys(i_rpCoordSys),
      m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_Extent(),
      m_Tolerance(HNumeric<double>::GLOBAL_EPSILON()),
      m_AutoToleranceActive(true)
    {
    m_Points.reserve(i_rCapacity);
    // Default tolerance is used
    ResetTolerance();

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system and initial list of points)
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::HVE3DPolyLine(const HGF3DPointCollection& i_rListOfPoints,
                                    const HFCPtr<HGF2DCoordSys>& i_rpCoordSys)
    : m_pCoordSys(i_rpCoordSys),
      m_Points(i_rListOfPoints),
      m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_Extent(),
      m_Tolerance(HNumeric<double>::GLOBAL_EPSILON()),
      m_AutoToleranceActive(true)

    {
    // Default tolerance is used
    ResetTolerance();

    HINVARIANTS;
    }


#if (0)
//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE3DPolyLine object.
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::HVE3DPolyLine(const HGF3DPoint& i_rFirstPoint,
                                    const HGF3DPoint& i_rSecondPoint)
    : m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_Extent(),
      m_Tolerance(0.0),
      m_AutoToleranceActive(true)

    {
    // The start and end points are set ... all we need is to copy them\
    // to the positions list
    m_Points.push_back(m_StartPoint.GetPosition());
    m_Points.push_back(pi_rSecondPoint.ExpressedIn(GetCoordSys()).GetPosition());

    // Set tolerance
    ResetTolerance();

    HINVARIANTS;
    }
#endif

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE3DPolyLine object.
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::HVE3DPolyLine(const HVE3DPolyLine& i_rObj)
    : m_Points(i_rObj.m_Points),
      m_pCoordSys(i_rObj.m_pCoordSys),
      m_ExtentUpToDate(i_rObj.m_ExtentUpToDate),
      m_Extent(i_rObj.m_Extent),
      m_2DPolyLineUpToDate(i_rObj.m_2DPolyLineUpToDate),
      m_p2DPolyLine(i_rObj.m_p2DPolyLine),
      m_Tolerance(i_rObj.m_Tolerance),
      m_AutoToleranceActive(i_rObj.m_AutoToleranceActive)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE3DPolyLine::~HVE3DPolyLine()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polysegment.
//-----------------------------------------------------------------------------
inline HVE3DPolyLine& HVE3DPolyLine::operator=(const HVE3DPolyLine& i_rObj)
    {
    // Check that given is not self
    if (&i_rObj != this)
        {
        // Copy attributes
        m_Points              = i_rObj.m_Points;
        m_ExtentUpToDate      = i_rObj.m_ExtentUpToDate;
        m_Extent              = i_rObj.m_Extent;
        m_2DPolyLineUpToDate  = i_rObj.m_2DPolyLineUpToDate;
        m_p2DPolyLine         = i_rObj.m_p2DPolyLine;
        m_Tolerance           = i_rObj.m_Tolerance;
        m_AutoToleranceActive = i_rObj.m_AutoToleranceActive;


        HINVARIANTS;
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// SetCoordSys
// Sets the coordinate system
//-----------------------------------------------------------------------------
inline void HVE3DPolyLine::SetCoordSys(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys)
    {
    m_pCoordSys = i_rpCoordSys;
    }

//-----------------------------------------------------------------------------
// GetCoordSys
// Gets the coordinate system
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HVE3DPolyLine::GetCoordSys()
    {
    return m_pCoordSys;
    }

//-----------------------------------------------------------------------------
// AppendPoint
// Adds a point at the end of polyline
//-----------------------------------------------------------------------------
inline void HVE3DPolyLine::AppendPoint(const HGF3DPoint& i_rNewPoint)
    {
    HINVARIANTS;

    // Add new point at end
    m_Points.push_back(i_rNewPoint);

    // Indicate 2d polyline is not up to date
    m_2DPolyLineUpToDate = false;

    m_ExtentUpToDate = false;

    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// GetPoint
// Returns the point designated by index
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HVE3DPolyLine::GetPoint(size_t i_Index) const
    {
    HINVARIANTS;

    // The given index must be valid
    HPRECONDITION(i_Index < m_Points.size());

    // Return point in self coordinate system
    return(m_Points[i_Index]);
    }


//-----------------------------------------------------------------------------
// GetSize
// Returns the number of points in polysegment
//-----------------------------------------------------------------------------
inline size_t HVE3DPolyLine::GetSize() const
    {
    HINVARIANTS;

    return(m_Points.size());
    }


//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the polyline
//-----------------------------------------------------------------------------
inline HVE3DPolyLine* HVE3DPolyLine::Clone() const
    {
    HINVARIANTS;

    return(new HVE3DPolyLine(*this));
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the polyline is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE3DPolyLine::IsNull() const
    {
    HINVARIANTS;

    return(m_Points.size() == 0);
    }


//-----------------------------------------------------------------------------
// GetStartPoint
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HVE3DPolyLine::GetStartPoint() const
    {
    return(m_Points[0]);
    }


//-----------------------------------------------------------------------------
// GetEndPoint
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HVE3DPolyLine::GetEndPoint() const
    {
    return(m_Points[m_Points.size() -1]);
    }


//-----------------------------------------------------------------------------
// GetTolerance
//-----------------------------------------------------------------------------
inline double HVE3DPolyLine::GetTolerance() const
    {
    return(m_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetTolerance
//-----------------------------------------------------------------------------
inline void HVE3DPolyLine::SetTolerance(double i_Tolerance)
    {
    HPRECONDITION(i_Tolerance >= 0.0);

    m_Tolerance = i_Tolerance;

    // Indicate 2d polyline is not up to date
    m_2DPolyLineUpToDate = false;

    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// IsAutoToleranceActive
//-----------------------------------------------------------------------------
inline bool HVE3DPolyLine::IsAutoToleranceActive() const
    {
    return(m_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetAutoToleranceActive
//-----------------------------------------------------------------------------
inline void HVE3DPolyLine::SetAutoToleranceActive(bool i_AutoToleranceActive)
    {
    m_AutoToleranceActive = i_AutoToleranceActive;

    // Indicate 2d polyline is not up to date
    m_2DPolyLineUpToDate = false;

    ResetTolerance();
    }
END_IMAGEPP_NAMESPACE

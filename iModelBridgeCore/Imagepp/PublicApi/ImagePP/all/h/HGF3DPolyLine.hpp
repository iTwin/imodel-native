//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DPolyLine.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF3DPolyLine::HGF3DPolyLine()
    : m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_AutoToleranceActive(true),
      m_Tolerance(HNumeric<double>::GLOBAL_EPSILON())
    {
    ResetTolerance();

    HINVARIANTS;
    }



//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system and initial list of points)
//-----------------------------------------------------------------------------
inline HGF3DPolyLine::HGF3DPolyLine(const HGF3DPointCollection& i_rListOfPoints)
    : m_Points(i_rListOfPoints),
      m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_Extent(i_rpCoordSys),
      m_Tolerance(HNumeric<double>::GLOBAL_EPSILON()),
      m_AutoToleranceActive(true)

    {
    // Default tolerance is used
    ResetTolerance();

    HINVARIANTS;
    }


#if (0)
//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF3DPolyLine object.
//-----------------------------------------------------------------------------
inline HGF3DPolyLine::HGF3DPolyLine(const HGF3DPoint& i_rFirstPoint,
                                    const HGF3DPoint& i_rSecondPoint)
    : m_ExtentUpToDate(false),
      m_2DPolyLineUpToDate(false),
      m_Extent(pi_rFirstPoint.GetCoordSys()),
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
// Copy constructor.  It duplicates another HGF3DPolyLine object.
//-----------------------------------------------------------------------------
inline HGF3DPolyLine::HGF3DPolyLine(const HGF3DPolyLine& i_rObj)
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
inline HGF3DPolyLine::~HGF3DPolyLine()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polysegment.
//-----------------------------------------------------------------------------
inline HGF3DPolyLine& HGF3DPolyLine::operator=(const HGF3DPolyLine& i_rObj)
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
// AppendPoint
// Adds a point at the end of polyline
//-----------------------------------------------------------------------------
inline void HGF3DPolyLine::AppendPoint(const HGF3DPoint& i_rNewPoint)
    {
    HINVARIANTS;

    // Add new point at end
    m_Points.push_back(i_rNewPoint);

    // Indicate 2d polyline is not up to date
    m_2DPolyLineUpToDate = false;

    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// GetPoint
// Returns the point designated by index
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HGF3DPolyLine::GetPoint(size_t i_Index) const
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
inline size_t HGF3DPolyLine::GetSize() const
    {
    HINVARIANTS;

    return(m_Points.size());
    }


//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the polyline
//-----------------------------------------------------------------------------
inline HGF3DPolyLine* HGF3DPolyLine::Clone() const
    {
    HINVARIANTS;

    return(new HGF3DPolyLine(*this));
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the polyline is null (no length)
//-----------------------------------------------------------------------------
inline bool HGF3DPolyLine::IsNull() const
    {
    HINVARIANTS;

    return(m_Points.size() == 0);
    }


//-----------------------------------------------------------------------------
// GetStartPoint
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HGF3DPolyLine::GetStartPoint() const
    {
    return(m_Points[0]);
    }


//-----------------------------------------------------------------------------
// GetEndPoint
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HGF3DPolyLine::GetEndPoint() const
    {
    return(m_Points[m_Points.size() -1]);
    }


//-----------------------------------------------------------------------------
// GetTolerance
//-----------------------------------------------------------------------------
inline double HGF3DPolyLine::GetTolerance() const
    {
    return(m_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetTolerance
//-----------------------------------------------------------------------------
inline void HGF3DPolyLine::SetTolerance(double i_Tolerance)
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
inline bool HGF3DPolyLine::IsAutoToleranceActive() const
    {
    return(m_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetAutoToleranceActive
//-----------------------------------------------------------------------------
inline void HGF3DPolyLine::SetAutoToleranceActive(bool i_AutoToleranceActive)
    {
    m_AutoToleranceActive = i_AutoToleranceActive;

    // Indicate 2d polyline is not up to date
    m_2DPolyLineUpToDate = false;

    ResetTolerance();
    }
END_IMAGEPP_NAMESPACE
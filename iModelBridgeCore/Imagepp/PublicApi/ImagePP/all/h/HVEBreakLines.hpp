//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEBreakLines.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HGFBreaklineDefect and HVEBreakLines
//-----------------------------------------------------------------------------


//=============================================================================
// HGFBreaklineDefect
//=============================================================================

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect::HVEBreakLineDefect()
    : m_Code(0)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect::HVEBreakLineDefect(long i_DefectCode,
                                              const HGF3DCoord<double>& i_rDefectLocation,
                                              long i_BreakLineIndex)
    : m_Code(i_DefectCode),
      m_Location(i_rDefectLocation),
      m_BreakLineIndex(i_BreakLineIndex)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect::HVEBreakLineDefect(long i_DefectCode,
                                              const HGF3DCoord<double>& i_rDefectLocation)
    : m_Code(i_DefectCode),
      m_Location(i_rDefectLocation),
      m_BreakLineIndex(-1)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect::HVEBreakLineDefect(const HVEBreakLineDefect& i_rDefect)
    : m_Code(i_rDefect.m_Code),
      m_Location(i_rDefect.m_Location),
      m_BreakLineIndex(i_rDefect.m_BreakLineIndex)
    {
    }

//-----------------------------------------------------------------------------
// Destroyer
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect::~HVEBreakLineDefect()
    {
    // Nothing to do
    }

//-----------------------------------------------------------------------------
// Assignment operator
//-----------------------------------------------------------------------------
inline HVEBreakLineDefect& HVEBreakLineDefect::operator=(const HVEBreakLineDefect& i_rDefect)
    {
    // Verify that defect provided is not self
    if (this != &i_rDefect)
        {
        // Copy members
        m_Code           = i_rDefect.m_Code;
        m_Location       = i_rDefect.m_Location;
        m_BreakLineIndex = i_rDefect.m_BreakLineIndex;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// Data extraction (code)
//-----------------------------------------------------------------------------
inline long HVEBreakLineDefect::GetCode() const
    {
    return(m_Code);
    }


//-----------------------------------------------------------------------------
// Data extraction (location)
//-----------------------------------------------------------------------------
inline const HGF3DCoord<double>& HVEBreakLineDefect::GetLocation() const
    {
    return(m_Location);
    }

//-----------------------------------------------------------------------------
// PRIVATE METHOD (Accessible by friend class HVEBreakLines)
// Data extraction (breakline index)
//-----------------------------------------------------------------------------
inline long HVEBreakLineDefect::GetBreakLineIndex() const
    {
    return(m_BreakLineIndex);
    }




//=============================================================================
// HVEBreaklines
//=============================================================================


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLines::HVEBreakLines()
    : m_Tolerance(0.0),
      m_pCoordSys(0)
    {
    }


//-----------------------------------------------------------------------------
// Constructor with coordinate system only
//-----------------------------------------------------------------------------
inline HVEBreakLines::HVEBreakLines(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys)
    : m_Tolerance(0.0),
      m_pCoordSys(i_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// Constructor with coordinate system only
//-----------------------------------------------------------------------------
inline HVEBreakLines::HVEBreakLines(const HFCPtr<HGF2DCoordSys>& i_rpCoordSys,
                                    double i_Tolerance)
    : m_Tolerance(i_Tolerance),
      m_pCoordSys(i_rpCoordSys)
    {
    // Make sure the tolerance is greater than 0.0
    HPRECONDITION(m_Tolerance > 0.0);
    }

#if (0)

//-----------------------------------------------------------------------------
// Full Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLines::HVEBreakLines(const HGF3DPointWithFlagCollection& i_rDTMPoints)
    : m_Tolerance(0.0),
      m_CoordSys
    {
    }

#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVEBreakLines::HVEBreakLines(const HVEBreakLines& i_rObj)
    : m_BreakLineList(i_rObj.m_BreakLineList),
      m_DTMPoints(i_rObj.m_DTMPoints),
      m_Tolerance(i_rObj.m_Tolerance),
      m_pCoordSys(i_rObj.m_pCoordSys)
    {
    }



//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HVEBreakLines::~HVEBreakLines()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another position object.
//-----------------------------------------------------------------------------
inline HVEBreakLines& HVEBreakLines::operator=(const HVEBreakLines& i_rObj)
    {
    if (this != &i_rObj)
        {
        m_BreakLineList = i_rObj.m_BreakLineList;
        m_DTMPoints = i_rObj.m_DTMPoints;
        m_Tolerance = i_rObj.m_Tolerance;
        m_pCoordSys = i_rObj.m_pCoordSys;
        }
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetCoordSys
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DCoordSys>& HVEBreakLines::GetCoordSys() const
    {
    return(m_pCoordSys);
    }


//-----------------------------------------------------------------------------
// AddPoint
// Adds a new point to the structure
//-----------------------------------------------------------------------------
inline void HVEBreakLines::AddPoint(const HGF3DPoint& i_rNewPoint)
    {
    // Add point to list of points
    m_DTMPoints.push_back(i_rNewPoint);
    }

//-----------------------------------------------------------------------------
// AddLine
// Adds a new point to the structure
//-----------------------------------------------------------------------------
inline void HVEBreakLines::AddLine(const HVE3DPolyLine& i_rNewLine)
    {
    // Add point to list of points
    m_BreakLineList.push_back(i_rNewLine.Clone());
    }



//-----------------------------------------------------------------------------
// GetLineSize
// Returns the number of lines in breakline list
//-----------------------------------------------------------------------------
inline size_t HVEBreakLines::GetLineSize() const
    {
    return(m_BreakLineList.size());
    }

//-----------------------------------------------------------------------------
// GetPointSize
// Returns the number of points in breaklines structure.
//-----------------------------------------------------------------------------
inline size_t HVEBreakLines::GetPointSize() const
    {
    return(m_DTMPoints.size());
    }


//-----------------------------------------------------------------------------
// GetLine
// Returns the line designated from the breakline list
// The designation is a number that must be greater than 0 and smaller that
// the value returned by GetLineSize()
//-----------------------------------------------------------------------------
inline const HFCPtr<HVE3DPolyLine>& HVEBreakLines::GetLine(size_t i_LineIndex) const
    {
    // Make sure the index provided is valid.
    HPRECONDITION(i_LineIndex < m_BreakLineList.size());

    // Return polyline
    return(m_BreakLineList[i_LineIndex]);
    }


//-----------------------------------------------------------------------------
// GetPoint
// Returns the point designated from the point list
// The designation is a number that must be greater than 0 and smaller that
// the value returned by GetPointSize()
//-----------------------------------------------------------------------------
inline const HGF3DPoint& HVEBreakLines::GetPoint(size_t i_PointIndex) const
    {
    // Make sure the index provided is valid.
    HPRECONDITION(i_PointIndex < m_DTMPoints.size());

    // Return designated point
    return(m_DTMPoints[i_PointIndex]);
    }





















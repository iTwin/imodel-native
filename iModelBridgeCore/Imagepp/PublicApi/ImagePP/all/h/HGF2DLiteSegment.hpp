//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLiteSegment.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DLiteSegment::HGF2DLiteSegment()
    : m_Tolerance(HGLOBAL_EPSILON)
    {
    }

//-----------------------------------------------------------------------------
// Constructor (by two points)
// Tolerance is optional and defaults to HGLOBAL_EPSILON
//-----------------------------------------------------------------------------
inline HGF2DLiteSegment::HGF2DLiteSegment(const HGF2DPosition& pi_rStartPoint,
                                          const HGF2DPosition& pi_rEndPoint,
                                          double              pi_Tolerance)
    : m_StartPoint(pi_rStartPoint),
      m_EndPoint(pi_rEndPoint),
      m_Tolerance(pi_Tolerance)
    {
    // The tolerance may not be null nor negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // The tolerance may not be too great
    HPRECONDITION(pi_Tolerance <= HMAX_EPSILON);
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DLiteSegment object.
//-----------------------------------------------------------------------------
inline HGF2DLiteSegment::HGF2DLiteSegment(const HGF2DLiteSegment& pi_rObj)
    : m_StartPoint(pi_rObj.m_StartPoint),
      m_EndPoint(pi_rObj.m_EndPoint),
      m_Tolerance(pi_rObj.m_Tolerance)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DLiteSegment::~HGF2DLiteSegment()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another lite segment object.
//-----------------------------------------------------------------------------
inline HGF2DLiteSegment& HGF2DLiteSegment::operator=(const HGF2DLiteSegment& pi_rObj)
    {
    // Check that given is not self
    if (this != &pi_rObj)
        {
        // Copy attributes
        m_StartPoint = pi_rObj.m_StartPoint;
        m_EndPoint = pi_rObj.m_EndPoint;
        m_Tolerance = pi_rObj.m_Tolerance;
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance of the segment
//-----------------------------------------------------------------------------
inline void HGF2DLiteSegment::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null nor negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // The tolerance may not be too great
    HPRECONDITION(pi_Tolerance <= HMAX_EPSILON);

    m_Tolerance = pi_Tolerance;
    }


/** -----------------------------------------------------------------------------
    This method sets the start point of the segment.

    @param pi_rNewStartPoint Constant reference to the HGF2DPosition object
                             containing the definition of the new first point
                             of segment..

    Example:
    @code
        HGF2DLiteSegment        MySeg1();
        HGF2DPosition           MyStartPoint(10, 10);
        MySeg1.SetStartPoint(MyStartPoint);
    @end

    @see GetStartPoint()
    @see SetEndPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline void HGF2DLiteSegment::SetStartPoint(const HGF2DPosition& pi_rNewStartPoint)
    {
    m_StartPoint = pi_rNewStartPoint;
    }



/** -----------------------------------------------------------------------------
    This method sets the end point of the segment.

    @param pi_rNewEndPoint Constant reference to the HGF2DPosition object
                           containing the definition of the new end point
                           of segment..

    Example:
    @code
        HGF2DLiteSegment        MySeg1();
        HGF2DPosition           MyEndPoint(10, 10);
        MySeg1.SetEndPoint(MyEndPoint);
    @end

    @see GetEndPoint()
    @see SetStartPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline void HGF2DLiteSegment::SetEndPoint(const HGF2DPosition& pi_rNewEndPoint)
    {
    m_EndPoint = pi_rNewEndPoint;
    }


/** -----------------------------------------------------------------------------
    This method returns the current segment tolerance.

    @return The tolerance of the segment.
    -----------------------------------------------------------------------------
*/
inline double HGF2DLiteSegment::GetTolerance() const
    {
    return(m_Tolerance);
    }

/** -----------------------------------------------------------------------------
    This method returns the start point.

    @return A const reference to the internal start point of the segment.

    @see GetEndPoint()
    @see SetStartPoint()
    -----------------------------------------------------------------------------
*/
inline const HGF2DPosition& HGF2DLiteSegment::GetStartPoint() const
    {
    return(m_StartPoint);
    }

/** -----------------------------------------------------------------------------
    This method returns the end point.

    @return A const reference to the internal end point of the segment.

    @see GetStartPoint()
    @see SetEndPoint()
    -----------------------------------------------------------------------------
*/
inline const HGF2DPosition& HGF2DLiteSegment::GetEndPoint() const
    {
    return(m_EndPoint);
    }


/** -----------------------------------------------------------------------------
    This method determines if the two segment cross each others or not.

    @param pi_rSegment IN Constant reference to other segment object.

    @return true if the segment cross, and false otherwise.

    @see IntersectSegment()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLiteSegment::Crosses(const HGF2DLiteSegment& pi_rSegment) const
    {
    HGF2DPosition   DummyPoint;
    return (IntersectSegment(pi_rSegment, &DummyPoint) == HGF2DLiteSegment::CROSS_FOUND);
    }

/** -----------------------------------------------------------------------------
    This method checks if the two segment objects connect at their extremity point
    or not. To be linked, two segment must share one extremity point.
    The method does not check if the segment crosses at other points.

    @param pi_rSegment IN Constant reference to the HGF2DLiteSegment to determine
                          link with.

    @return true if the segment link, and false otherwise.

    -----------------------------------------------------------------------------
*/
inline bool   HGF2DLiteSegment::LinksTo(const HGF2DLiteSegment& pi_rSegment) const
    {
    // For two linear to connect,
    // they must have one extremity point equal to either extremity
    // point of the other

    // Tolerance is smallest of the two

    double Tolerance = MIN(m_Tolerance, pi_rSegment.m_Tolerance);

    return ((m_EndPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance)) ||
            (m_EndPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance)) ||
            (m_StartPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance)) ||
            (m_StartPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance)));
    }

END_IMAGEPP_NAMESPACE
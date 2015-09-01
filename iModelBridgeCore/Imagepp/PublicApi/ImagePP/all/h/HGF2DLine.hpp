//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLine.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    Constructor by mathematical parameters

    @param pi_rpCoordSys IN Constant reference to a smart pointer to a coordinate
                            system in which the line will be interpreted.

    @param pi_Slope IN The slope of the line. A vertical line may not be created
                       when using such constructor.

    @param pi_rIntercept IN The intercept of the line.


    -----------------------------------------------------------------------------
*/
inline HGF2DLine::HGF2DLine(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                            double                       pi_Slope,
                            double                       pi_Intercept)
    : m_Intercept(pi_Intercept),
      m_Slope(pi_Slope),
      m_InvertSlope(false),
      m_pCoordSys(pi_rpCoordSys)
    {
    // Check if the slope is too high
    if (m_Slope > 2.0)
        {
        // The slope is too high ... we will use the inverted slope instead
        m_Slope = 1.0 / pi_Slope;

        // Indicate the slope is inverted
        m_InvertSlope = true;

        // We need to find the inverted intercept
        m_Intercept = -m_Intercept / pi_Slope;
        }

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DLine object.
//-----------------------------------------------------------------------------
inline HGF2DLine::HGF2DLine(const HGF2DLine& pi_rObj)
    : m_Intercept(pi_rObj.m_Intercept),
      m_Slope(pi_rObj.m_Slope),
      m_InvertSlope(pi_rObj.m_InvertSlope),
      m_pCoordSys(pi_rObj.m_pCoordSys)
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DLine::~HGF2DLine()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HGF2DLine& HGF2DLine::operator=(const HGF2DLine& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_Intercept = pi_rObj.m_Intercept;
        m_Slope = pi_rObj.m_Slope;
        m_InvertSlope = pi_rObj.m_InvertSlope;
        m_pCoordSys = pi_rObj.m_pCoordSys;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// operator!=
// See operator== : returns opposite value.
//-----------------------------------------------------------------------------
inline bool HGF2DLine::operator!=(const HGF2DLine& pi_rObj) const
    {
    HINVARIANTS;
    return (!operator==(pi_rObj));
    }


/** -----------------------------------------------------------------------------
    This method returns the intercept with the Y axis of the line. If the line
    is vertical, then the returned value is the intercept with the X axis.
    To know if a line is vertical, the IsVertical() method should be used.
    The units of the distance returned are the units of the Y dimension of the
    used coordinate system, or of the X dimension if the line is vertical.

    @return A double containing the intercept

    @see IsVertical()
    @see GetSlope()
    -----------------------------------------------------------------------------
*/
inline double HGF2DLine::GetIntercept() const
    {
    HINVARIANTS;

    double ReturnValue;

    // Check if we have the inverted slope
    if (m_InvertSlope)
        {
        // Check if the slope is different from 0.0
        if (m_Slope == 0)
            {
            // We have a vertical line ... we return the X intercept
            ReturnValue = m_Intercept;
            }
        else
            {
            ReturnValue = -m_Intercept / m_Slope;
            }
        }
    else
        {
        // The slope is not inverted ... we return the intercept as it is
        ReturnValue = m_Intercept;
        }

    return (ReturnValue);
    }

/** -----------------------------------------------------------------------------
    This method returns the slope of the line. If the line is vertical, then the
    returned value is undefined. To know if a line is vertical, the IsVertical()
    method should be used.

    @return The slope of the line

    @see IsVertical()
    @see GetIntercept()
    -----------------------------------------------------------------------------
*/
inline double HGF2DLine::GetSlope () const
    {
    HINVARIANTS;

    double ReturnValue;

    if (m_InvertSlope)
        {
        // The slope is inverted ... check if it is null
        if (m_Slope == 0.0)
            {
            // We have a vertical line ... the slope should not be asked but we will return 0 as a service
            ReturnValue = 0.0;
            }
        else
            {
            // The line is not vertical ... return inverse of inverted slope
            ReturnValue = (1.0 / m_Slope);
            }
        }
    else
        ReturnValue = m_Slope;

    return(ReturnValue);
    }

/** -----------------------------------------------------------------------------
    This method returns true if the line is vertical, and false otherwise.

    @return true if the line is vertical, and false otherwise.

    @see GetSlope()
    @see GetIntercept()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLine::IsVertical () const
    {
    HINVARIANTS;

    return(m_InvertSlope && m_Slope == 0.0);
    }

/** -----------------------------------------------------------------------------
    This method permits to get the coordinate system currently used in the
    interpretation of the line.

    @return A constant reference to a smart pointer to the HGF2DCoordSys instance
            used in the interpretation of the line.

    @see SetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DCoordSys>& HGF2DLine::GetCoordSys () const
    {
    HINVARIANTS;
    return(m_pCoordSys);
    }

/** -----------------------------------------------------------------------------
    Modifies the interpretation coordinate system of the line. Mathematical
    values of the definitions are unchanged.

    @see GetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLine::SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    HINVARIANTS;
    // Set coordinate system
    m_pCoordSys = pi_rpCoordSys;
    }


END_IMAGEPP_NAMESPACE
//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DVector.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DVector::HVE2DVector()
    : HGFGraphicObject(),
      m_Tolerance(HGLOBAL_EPSILON),
      m_pStrokeTolerance(),
      m_AutoToleranceActive(true)
    {
    }

//-----------------------------------------------------------------------------
// Constructor with coordinate syste,
//-----------------------------------------------------------------------------
inline HVE2DVector::HVE2DVector(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HGFGraphicObject(pi_rpCoordSys),
      m_Tolerance(HGLOBAL_EPSILON),
      m_pStrokeTolerance(),
      m_AutoToleranceActive(true)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DVector object.
//-----------------------------------------------------------------------------
inline HVE2DVector::HVE2DVector(const HVE2DVector& pi_rObj)
    : HGFGraphicObject(pi_rObj),
      m_Tolerance(pi_rObj.m_Tolerance),
      m_AutoToleranceActive(pi_rObj.m_AutoToleranceActive)
    {
    if (pi_rObj.m_pStrokeTolerance != NULL)
        m_pStrokeTolerance = new HGFTolerance(*pi_rObj.m_pStrokeTolerance);
    else
        m_pStrokeTolerance = NULL;

    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DVector::~HVE2DVector()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another vector object.
//-----------------------------------------------------------------------------
inline HVE2DVector& HVE2DVector::operator=(const HVE2DVector& pi_rObj)
    {
    // Check that object is not self
    if (this != &pi_rObj)
        {
        HGFGraphicObject::operator=(pi_rObj);
        m_Tolerance = pi_rObj.m_Tolerance;
        m_AutoToleranceActive = pi_rObj.m_AutoToleranceActive;

        if (pi_rObj.m_pStrokeTolerance != NULL)
            m_pStrokeTolerance = new HGFTolerance(*pi_rObj.m_pStrokeTolerance);
        else
            m_pStrokeTolerance = NULL;
        }

    // Return reference to self
    return (*this);
    }

#if (0)
//-----------------------------------------------------------------------------
// Touches
// This method returs true if self and the given vector are not disjoint from
// each other
//-----------------------------------------------------------------------------
inline bool HVE2DVector::Touches(const HVE2DVector& pi_rVector) const
    {
    // First we check if the extent of our objects are disjoint and if not,
    // we check if they do not cross, flirt or are contiguous to one another
    return ((!GetExtent().DoTheyOverlap(pi_rVector.GetExtent())) &&
            (Crosses(pi_rVector) || Flirts(pi_rVector) || AreContiguous(pi_rVector)));
    }
#endif


/** -----------------------------------------------------------------------------
    This method returns the tolerance of the vector.

    @return A number indicating the tolerance.

    Example
    @code
    @end

    @see SetTolerance()
    @see IsAutoToleranceActive()
    @see SetAutoToleranceActive()
    -----------------------------------------------------------------------------
*/
inline double HVE2DVector::GetTolerance() const
    {
    return(m_Tolerance);
    }

/** -----------------------------------------------------------------------------
    This method sets the tolerance of the vector. In order to do this
    the AUTO TOLERANCE setting should be previously set to false, otherwise
    setting of tolerance will be effective only till the vector is modified
    or tolerance is recomputed for any other reason. The duration of the
    tolerance setting in such case is unpredictible. Some vector
    types may completely ignore the tolerance setting is AUTO TOLERANCE
    determination is active.

    @param pi_Tolerance The new tolerance value.

    Example
    @code
    @end

    @see GetTolerance()
    @see IsAutoToleranceActive()
    @see SetAutoToleranceActive()
    -----------------------------------------------------------------------------
*/
inline void HVE2DVector::SetTolerance(double pi_Tolerance)
    {
    // The given tolerance may not be zero nor smaller
    HPRECONDITION(pi_Tolerance > 0.0);

    // The given tolerance must be smaller or equal than maximum epsilon
    HPRECONDITION(pi_Tolerance <= HMAX_EPSILON);

    m_Tolerance = pi_Tolerance;
    }

/** -----------------------------------------------------------------------------
    This indicates if automatic tolerance determination is active. If
    set to automatic, the tolerance is automatically determined by the
    vector upon the value of the coordinate of this vector.

    @return true if automatic tolerance determination is active
            and false otherwise.

    Example
    @code
    @end

    @see SetTolerance()
    @see GetTolerance()
    @see SetAutoToleranceActive()
    -----------------------------------------------------------------------------
*/
inline bool HVE2DVector::IsAutoToleranceActive() const
    {
    return(m_AutoToleranceActive);
    }

/** -----------------------------------------------------------------------------
    This method sets the automatic tolerance determination of the vector.
    If set, the vector will automatically determine the most appropriate
    tolerance applicable to current coordinates. If unset, the tolerance
    is left unchanged, but can be manually modified with the
    SetTolerance() method.

    @param pi_AutoToleranceActive If true, then auto tolerance will from now
                                  on be active. If false, then autotolerance
                                  will be from now on inactive.

    Example
    @code
    @end

    @see GetTolerance()
    @see IsAutoToleranceActive()
    @see SetTolerance()
    -----------------------------------------------------------------------------
*/
inline void HVE2DVector::SetAutoToleranceActive(bool pi_ActiveAutoTolerance)
    {
    m_AutoToleranceActive = pi_ActiveAutoTolerance;
    }


//-----------------------------------------------------------------------------
// public
// Locate
//-----------------------------------------------------------------------------
inline HGFGraphicObject::Location HVE2DVector::Locate(const HGF2DLocation& pi_rPoint) const
    {
    return(IsPointOn(pi_rPoint, INCLUDE_EXTREMITIES) ? S_ON_BOUNDARY : S_OUTSIDE);
    }

END_IMAGEPP_NAMESPACE
